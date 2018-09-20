package API;


=pod
SmartHome WiFi Bridge garage door API.

The status file should have the following format:
    door: opened | closed | opening | closing | blocked
    interior lights: on | off
    exterior lights: on | off

The command file should have the following format, corresponding to:
#   door            exterior lights     interior lights
    open | close    on | off            on | off
=cut


use strict;
use warnings;


use Readonly;
use Time::HiRes qw(usleep);


# Constants


Readonly my $COMMAND_FILE => 'command.txt';
Readonly my $COMMAND_LOCK_FILE => 'command.lock';
Readonly my $STATUS_FILE => 'status.txt';
Readonly my $STATUS_LOCK_FILE => 'status.lock';

Readonly my $COMMAND_FILE_WAIT_INTERVAL => 100; # ms
Readonly my $STATUS_FILE_WAIT_INTERVAL => 50; # ms

Readonly my @KNOWN_KEYS => ('door', 'interior lights', 'exterior lights');
Readonly my %KNOWN_KEYS => map { $_ => 1 } @KNOWN_KEYS;


# Interface


sub new {
    my ($class) = @_;
    my $this = {
            status => undef,
        };
    bless $this, $class;
}


sub sendCommand {
    my ($this, $door, $exteriorLights, $interiorLights) = @_;

    open my $lockFile, '>', $COMMAND_LOCK_FILE or die "Unable to open command lock file '$COMMAND_LOCK_FILE' for writing: $!\n";
    close $lockFile;

    open my $commandFile, '>', $COMMAND_FILE or die "Unable to open command file '$COMMAND_FILE' for writing: $!\n";
    print $commandFile "$door $exteriorLights $interiorLights\n";
    close $commandFile;

    unlink $lockFile or die "Unable to delete command lock file '$COMMAND_LOCK_FILE' for writing: $!\n";

    while (-f $COMMAND_FILE) {
        usleep($COMMAND_FILE_WAIT_INTERVAL);
    }
}


sub isExteriorLightsOn {
    my ($this) = @_;
    $this->_readStatus();
    return $this->{status}->{'exterior lights'} eq 'on';
}

sub isInteriorLightsOn {
    my ($this) = @_;
    $this->_readStatus();
    return $this->{status}->{'interior lights'} eq 'on';
}


sub getDoorStatus {
    my ($this) = @_;
    $this->_readStatus();
    return $this->{status}->{door};
}

sub isBlocked {
    my ($this) = @_;
    $this->_readStatus();
    return $this->{status}->{door} eq 'blocked';
}

sub isClosed {
    my ($this) = @_;
    $this->_readStatus();
    return $this->{status}->{door} eq 'closed';
}

sub isClosing {
    my ($this) = @_;
    $this->_readStatus();
    return $this->{status}->{door} eq 'closing';
}

sub isOpened {
    my ($this) = @_;
    $this->_readStatus();
    return $this->{status}->{door} eq 'opened';
}

sub isOpening {
    my ($this) = @_;
    $this->_readStatus();
    return $this->{status}->{door} eq 'opening';
}


# Implementation


sub _readStatus {
    my ($this) = @_;

    return if defined $this->{status};
    $this->{status} = {};

    while (-f $STATUS_LOCK_FILE || !-f $STATUS_FILE) {
        usleep($STATUS_FILE_WAIT_INTERVAL);
    }

    open my $file, '<', $STATUS_FILE or die "Unable to open status file '$STATUS_FILE' for reading: $!\n";
    foreach my $line (<$file>) {
        chomp $line;
        my ($key, $value) = $line =~ /^\s*([^:]+)\s*:\s*(.+)$/
            or die "Unable to parse status file '$STATUS_FILE' line '$line'\n";
        die "Duplicate key '$key' in status file '$STATUS_FILE'\n" if exists $this->{status}->{$key};
        $this->{status}->{$key} = $value; 
    }

    if (my @unknownKeys = grep { !exists $KNOWN_KEYS{$_} } sort keys %{$this->{status}}) {
        die "Unknown key(s) ".join(', ', map { "'$_'" } @unknownKeys)." in status file '$STATUS_FILE'\n";
    }
    if (my @missingKeys = grep { !exists $this->{status}->{$_} } sort @KNOWN_KEYS) {
        die "Missing key(s) ".join(', ', map { "'$_'" } @missingKeys)." in status file '$STATUS_FILE'\n";
    }
}


1;
