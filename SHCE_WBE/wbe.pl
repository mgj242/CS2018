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
use URI;

use Protocol::WebSocket::Client;


my $selfFileName = basename($0);

ReadMode 3; # cbreak
select STDIN;
$| = 1;
select STDOUT;
$| = 1;


# constants

Readonly my $MAX_BYTES_TO_READ => 1024;
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
Usage:  $selfFileName  smart_home_central_host:port/path door_stripes_count
EOM
}


my ($shcWsUrl, $doorStripesCount) = @ARGV;

die error('Door stripes count must be > 2') unless $doorStripesCount > 2;


# connect to the SmartHome central WebSocket

my ($host, $port, $path) = $shcWsUrl =~ m!^([^:]+):(\d+)/(.*)$!
    or die error('need host, port and optionally path');

my $sock = IO::Socket::INET->new(PeerAddr => $host,
        PeerPort => $port, Proto => 'tcp')
    or die error("unable to open socket to '$shcWsUrl': $@");

print "Connected to '$shcWsUrl'\n";


# WebSocket protocol command handling

my $currentDoorStripe = 1; # closed
my $motorState = 'stop';
my $exteriorLights = 0;
my $interiorLights = 0;

sub query() {
    return "$motorState, door stripe $currentDoorStripe of $doorStripesCount, exterior lights ".
        ($exteriorLights ? 'on' : 'off').', interior lights '.($interiorLights ? 'on' : 'off');
}

Readonly my %netCommands => (
        'full open' => sub {
                return 'WARNING already fully opened' if $currentDoorStripe == $doorStripesCount;
                return 'WARNNIG already fully opening' if $motorState eq 'full open';
                return 'ERROR motor is blocked' if $motorState eq 'blocked';
                $motorState = 'full open';
                return 'OK opening';
            },
        'step up' => sub {
                return 'WARNING already fully opened' if $currentDoorStripe == $doorStripesCount;
                return 'WARNNIG already fully opening' if $motorState eq 'step up';
                return 'ERROR motor is blocked' if $motorState eq 'blocked';
                $motorState = 'step up';
                return 'OK stepping up';
            },
        'full close' => sub {
                return 'WARNING already fully closed' if $currentDoorStripe == 1;
                return 'ERROR motor is blocked' if $motorState eq 'blocked';
                $motorState = 'full close';
                return 'OK closing';
            },
        'step down' => sub {
                return 'WARNING already fully opened' if $currentDoorStripe == $doorStripesCount;
                return 'ERROR motor is blocked' if $motorState eq 'blocked';
                $motorState = 'step down';
                return 'OK stepping down';
            },
        'stop' => sub {
                return 'WARNING already stopped' if $motorState eq 'stop';
                return 'ERROR motor is blocked' if $motorState eq 'blocked';
                $motorState = 'stop';
                return 'OK stopped';
            },
        'switch exterior lights on' => sub {
                return 'WARNING exterior lights are already on' if $exteriorLights;
                $exteriorLights = 1;
                return 'OK exterior lights switched on';
            },
        'switch exterior lights off' => sub {
                return 'WARNING exterior lights are already off' if !$exteriorLights;
                $exteriorLights = 0;
                return 'OK exterior lights switched off';
            },
        'switch interior lights on' => sub {
                return 'WARNING interior lights are already on' if $interiorLights;
                $interiorLights = 1;
                return 'OK interior lights switched on';
           },
        'switch interior lights off' => sub {
                return 'WARNING interior lights are already off' if !$interiorLights;
                $interiorLights = 0;
                return 'OK interior lights switched off';
            },
        'query' => sub { return query().', no message'; },
    );


# create WebSocket client

my $wsUrl = 'ws://'.$host.':'.$port.(length $path ? '/'.$path : '');
print "Connecting WebSocket client to '$wsUrl'\n";

my $continue = 1;
my $shouldWritePrompt = 1;

my $client = Protocol::WebSocket::Client->new(
        url => $wsUrl, version => $PROTOCOL_VERSION,
        on_connect => sub {
            my ($client) = @_;
            print "Handshake completed\n";
            $client->write('hello');
        },
        on_eof => sub {
            print "Server closed the connection\n";
            $continue = 0;
        },
        on_error => sub {
            my ($client, $error) = @_; 
            print STDERR error("WebSocket error: $error");
            $continue = 0;
        },
        on_frame => sub {
            my ($client, $message) = @_;
#            print "Received '$message'\n";
            my $handler = $netCommands{$message};
            if ($handler) {
                my $reply = $handler->();
#                print "Sending '$reply'\n";
                $client->write($reply);
            } else {
                $client->write("ERROR unknown command '$message'");
            }
            $shouldWritePrompt = 1;
       },
        on_write => sub {
            my ($client, $buf) = @_; 
            syswrite $sock, $buf;
#            print "RAW '$buf' sent\n";
        }
    );
 
# send handshake header, and parse incoming data 

$client->connect;
print "Handshake sent\n";


# start selecting for reading on socket

my $select = IO::Select->new;
$select->add($sock);

Readonly my %consoleCommands => (
        's' => sub { $motorState = 'stop'; },
        'U' => sub { $motorState = 'motor up'; },
        'D' => sub { $motorState = 'motor down'; },
        'u' => sub { $motorState = 'step up'; },
        'd' => sub { $motorState = 'step down'; },
        'b' => sub { $motorState = 'blocked'; },
        'l0' => sub { $exteriorLights = 0; },
        'l1' => sub { $exteriorLights = 1; },
        'L0' => sub { $interiorLights = 0; },
        'L1' => sub { $interiorLights = 1; },
        '-' => sub { --$currentDoorStripe if $currentDoorStripe > 1; },
        '+' => sub { ++$currentDoorStripe if $currentDoorStripe < $doorStripesCount; },
    );

# main loop

$SIG{INT} = sub {
    $continue = 0;
    ReadMode 0; # normal
};

my $command = '';
while ($continue) {
    my @ready;
    while ((@ready = $select->can_read($READ_SOCKET_WAIT_TIME_SEC)) && grep { $_ == $sock} @ready) {
#        print "Socket ready ...\n";
        my $buffer;
        my $bytesRead = sysread $sock, $buffer, $MAX_BYTES_TO_READ;
#        print "Socket red OK ...\n";
        if (!defined $bytesRead) {
            print STDERR error("reading from client socket failed: $!");
            last;
        }
        if ($bytesRead == 0) {
            print STDERR error('server unexpectedly closed the socket');
            last;
        }
#        print "Received RAW '$buffer'\n";
        $client->read($buffer);
    }

    if ($shouldWritePrompt) {
        print query().' > ';
        $shouldWritePrompt = 0;
    }

    if (defined (my $key = ReadKey(-1))) {
        print $key;
        if ($key eq "\r") {
            print "\n";
            if (length $command) {
                my $handler = $consoleCommands{$command};
                if ($handler) {
                    $handler->();
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
}

# send correct close header
$client->disconnect;
