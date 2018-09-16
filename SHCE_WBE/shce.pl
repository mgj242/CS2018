#!/usr/bin/env perl


use strict;
use warnings;


our $VERSION = '1.0.0';


use lib '3rd';

use Carp qw(confess);
use File::Basename;
use IO::Select;
use IO::Socket::INET;
use Readonly;
use Term::ReadKey;

use Protocol::WebSocket::Frame;
use Protocol::WebSocket::Handshake::Server;


my $selfFileName = basename($0);

ReadMode 3; # cbreak
select STDIN;
$| = 1;
select STDOUT;
$| = 1;


# constants

Readonly my $PROTOCOL_VERSION => 'draft-ietf-hybi-17';
Readonly my $READ_SOCKET_WAIT_TIME_SEC => 0.01;


# utilities

sub error($) {
    return "$selfFileName: ERROR: $_[0]\n";
}

sub warning($) {
    warn "$selfFileName: WARNING: $_[0]\n";
}

sub fatal($) {
    confess "$selfFileName: FATAL: $_[0]";
}


# parse command line arguments

unless (@ARGV == 2) {
    die <<EOM;
Usage:  $selfFileName  port path
EOM
}

my ($port, $path) = @ARGV;
$path = '/'.$path unless $path =~ m!^/!;


# create WebSocket server socket

my $sock = IO::Socket::INET->new(
        LocalHost => '127.0.0.1',
        LocalPort => $port,
        Proto     => 'tcp',
        PeerAddr  => inet_ntoa(INADDR_BROADCAST),
        Listen    => 2, # queue length
        Reuse     => 1)
    or die error("binding of socket to localhost:$port failed: $@");

print "Bound to local port $port\n";


# main loop

CONNECTION: while (1) {
    # wait for new client connection, then accept it
    my $clientSock = $sock->accept()
        or die error("accept() on socket bound to localhost:$port failed: $@");

    my ($clientAddr, $clientPort) = ($clientSock->peerhost(), $clientSock->peerport());
    print "Accepted new connection from $clientAddr:$clientPort\n";

    # perform WebSocket handshake

    my $handshake = Protocol::WebSocket::Handshake::Server->new;
    while (!$handshake->is_done) {
        my $line = <$clientSock>;
        unless (defined $line) {
            warning('client closed its socket unexpectedly');
            next CONNECTION;
        }
#        print "RAW handshake: '$line'\n";
        $handshake->parse($line);
        die error('WebSocket handshake failed') if $handshake->error;
    }

    my $requestedResource = $handshake->req->resource_name;
    if ($requestedResource ne $path) {
        print STDERR error("client requested resource '$requestedResource', but '$path' expected");
        $clientSock->close();
        next CONNECTION;
    }

    my $send = sub {
        my ($buffer) = @_;
#        print "Sending RAW '$buffer'\n";
        my $bytesWritten = 0;
        do {
            $bytesWritten = syswrite $clientSock, $buffer;
            if (!defined $bytesWritten) {
                print STDERR error("writing to client socket failed: $!");
                $clientSock->close();
                return 0;
            }
        } while ($bytesWritten <= 0);
        return 1;
    };

    $send->($handshake->to_string) or next CONNECTION;

    print "Handshake completed\n";

    # process messages from the client

    $_ = '?' for my ($motorState, $currentStripe, $stripesCount, $extLights, $intLights, $lastMessage);
    my $state = 'INITIAL';
    my $shouldWritePrompt = 1;
    my $command = "";

    Readonly my %states => (
            'INITIAL' => sub {
                    my ($message) = @_;
                    if ($message ne 'hello') {
                        print STDERR error("unexpected initial message '$message'");
                    }
                    return ('query', 'QUERY');
                },
            'NORMAL' => sub { return ('query', 'QUERY'); },
            'QUERY' => sub {
                    my ($message) = @_;
                    if (my ($severity, $text) = $message =~ /^(OK|WARNING|ERROR)\s+(.+)$/) {
                        print "Received: $severity $text\n";
                    } else {
                        ($motorState, $currentStripe, $stripesCount, $extLights, $intLights, $lastMessage) =
                            $message =~ /^(.+), door stripe (\d+) of (\d+), exterior lights (\w+), interior lights (\w+)(?:,\s+(.*))?$/
                            or die error("unable to parse query response '$message'");
                    }
                    return ('query', 'QUERY');
                },
            'RESPONSE' => sub {
                    my ($message) = @_;
                    print "Received: $message\n";
                    return ('query', 'QUERY');
                },
        );

    Readonly my %consoleCommands => (
            'x' => sub { return (undef, 'INITIAL'); },
            'q' => sub { return ('query', 'QUERY'); },
            's' => sub { return ('stop', 'RESPONSE'); },
            'U' => sub { return ('full open', 'RESPONSE'); },
            'D' => sub { return ('full close', 'RESPONSE'); },
            'u' => sub { return ('step up', 'RESPONSE'); },
            'd' => sub { return ('step down', 'RESPONSE'); },
            'l0' => sub { return ('switch exterior lights off', 'RESPONSE'); },
            'l1' => sub { return ('switch exterior lights on', 'RESPONSE'); },
            'L0' => sub { return ('switch interior lights off', 'RESPONSE'); },
            'L1' => sub { return ('switch interior lights on', 'RESPONSE'); },
        );

    # start selecting for reading on socket

    my $select = IO::Select->new;
    $select->add($clientSock);

    while (1) {
        my $request;

        my @ready;
        while ((@ready = $select->can_read($READ_SOCKET_WAIT_TIME_SEC)) && grep { $_ == $clientSock} @ready) {
#            print "Socket ready ...\n";
            my $buffer;
            my $bytesRead = sysread $clientSock, $buffer, 1024;
#            print "Socket red OK ...\n";
            if (!defined $bytesRead) {
                print STDERR error("reading from client socket failed: $!");
                $clientSock->close();
                next CONNECTION;
            }
            last if $bytesRead == 0;
#            print "Received RAW '$buffer'\n";
            my $frame = $handshake->build_frame(max_fragments_amount => 1024);
            $frame->append($buffer);
            while (defined(my $message = $frame->next_bytes)) {
#                print "Received '$message'\n";
                if ($frame->is_close) {
                    print "Final frame received, closing the connection\n";
                    $send->($handshake->build_frame(type => 'close', version => $PROTOCOL_VERSION)->to_bytes);
                    $clientSock->close();
                    next CONNECTION;
                }
                my $handler = $states{$state} or fatal("unexpected state '$state'");
                ($request, $state) = $handler->($message);
                $shouldWritePrompt = 1;
            }
        }

        if ($shouldWritePrompt) {
            print "($state) $motorState, $currentStripe/$stripesCount, E $extLights, I $intLights, $lastMessage > ";
            $shouldWritePrompt = 0;
        }

        if (defined (my $key = ReadKey(-1))) {
            print $key;
            if ($key eq "\r") {
                print "\n";
                if (length $command) {
                    my $handler = $consoleCommands{$command};
                    if ($handler) {
                        ($request, $state) = $handler->();
                        fatal('no state returned from handler') unless defined $state;
                        unless (defined $request) {
                            print STDERR "Closing the connection\n";
                            $send->($handshake->build_frame(type => 'close', version => $PROTOCOL_VERSION)->to_bytes);
                            $clientSock->close();
                            next CONNECTION;
                        }
                    } else {
                        print STDERR 'Unknown command, try one of: '.join(', ', sort keys %consoleCommands)."\n";
                    }
                }
                $command = '';
                $shouldWritePrompt = 1;
            } else {
                $command .= $key;
            }
        }

        if (defined $request) {
#            print "Current state: $state, sending request '$request'\n";
            my $frame = $handshake->build_frame(max_fragments_amount => 1024);
            $frame->append($request);
            $send->($frame->to_bytes) or next CONNECTION;
        }
    }

    $clientSock->close();
}
