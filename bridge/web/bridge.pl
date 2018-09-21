#!/usr/bin/env perl


use strict;
use warnings;


BEGIN { unshift @INC, '.'; }

use CGI qw(meta);
use Readonly;

use API;


# Constants


Readonly my %AUTH => ( 'postman' => 'pass' );

Readonly my $REFRESH_INTERVAL => 5; # seconds


# CSS and Javascript

my $css =<<EOC;
<!--
    .fieldset-auto-width {
        display: inline-block;
        width: 140px;
    }

    .error {
        color: red;
    }

    .ck-button {
        margin: 4px;
        background-color: #E4E4E4;
        border-radius: 4px;
        border: 1px solid #D0D0D0;
        overflow: auto;
    }
    .ck-button label {
        width: 4.0em;
    }
    .ck-button label span {
        text-align: center;
        padding: 3px 0px;
        display: block;
    }
    .ck-button label input {
        position: absolute;
        top: -20px;
    }

    .ck-button input:checked + span {
        background-color: #66B2FF;
        color: #FFFFFF;
    }
-->
EOC

my $script =<<EOC;
EOC


my $api = API->new;
my $query = CGI->new;


# check for authorization

my $authCookie;

$_ = 0 for my ($authorized, $authorizationFailed);
if ($authCookie = $query->cookie('auth')) {
    my ($name, $pass) = $authCookie->value();
    $authorized = defined $name && defined $pass && exists $AUTH{$name} && $AUTH{$name} eq $pass;
}

if (!$authorized) {
    my $name = $query->param('name');
    my $pass = $query->param('pass');
    $authorized = defined $name && defined $pass && exists $AUTH{$name} && $AUTH{$name} eq $pass;
    $authorizationFailed = (defined $name || defined $pass) & !$authorized;
    $authCookie = $query->cookie({ -name => 'auth', -value => [ $name, $pass ] }) if $authorized;
}


my @errors;


# send a command according to the requested action

if ($authorized && ($query->param('close') || $query->param('open'))) {
    eval {
        $api->sendCommand($query->param('close') ? 'close' : 'open',
                $query->param('exterior') ? 1 : 0,
                $query->param('interior') ? 1 : 0);
    };
    push @errors, $@ if length $@;
}


# HTML header and heading

my %htmlTagAttrs = (
        -title => 'Smart Home Garage Control Point',
        -style => { -code => $css }, -script => $script
    );
if ($authorized) {
    $htmlTagAttrs{-head} = meta({-http_equiv => 'refresh', -content => $REFRESH_INTERVAL});
    $query->delete('close', 'open');
}

print $query->header({ -type => 'text/html', -cookie => $authCookie }).
    $query->start_html(\%htmlTagAttrs).
    $query->start_center().
        $query->h1('Smart Home<br/>Garage').
        $query->img({ -src => 'fptLogo.png', -width => '155px' }).$query->br().$query->br().
        $query->start_form({ -method => 'POST', -action => 'bridge.pl', -enctype => 'multipart/form-data' });


# authorization fieldset

if (!$authorized) {
    print   $query->start_div().
                $query->start_fieldset({ class => 'fieldset-auto-width' }).$query->legend('Authorization').$query->br().
                    '<b>Name:</b>'.$query->br().$query->textfield({ -name => 'name', -size => 15 }).$query->br().$query->br().
                    '<b>Password:</b>'.$query->br().$query->password_field({ -name => 'password', -size => 15 });
    print   $query->br().$query->start_div({ -class => 'error' }).$query->p('Authorization failed!').$query->end_div()
        if $authorizationFailed;
    print       $query->end_fieldset().
            $query->end_div();
}


# operations fieldset

sub buttonCheckbox($$$) {
    my ($label, $name, $value) = @_;
    return  $query->start_div({ -class => 'ck-button' }).
                $query->start_label().
                    $query->input({ -type => 'checkbox', -name => $name, -value => ($value ? 'on' : 'off') }).
                    $query->span($label).
                $query->end_label().
            $query->end_div();
}

eval {
    print   $query->start_div().
                $query->start_fieldset({ class => 'fieldset-auto-width' }).$query->legend('Operations').$query->br().
                    buttonCheckbox('Interior lights', 'interior', $api->isInteriorLightsOn()).
                    buttonCheckbox('Exterior lights', 'exterior', $api->isExteriorLightsOn()).
                    $query->br();

    print           $query->submit({ -name => 'close', -value => 'Close door' })
        if $api->isOpened() || $api->isOpening();
    print           $query->submit({ -name => 'open', -value => 'Open door' })
        if $api->isClosed() || $api->isClosing();

    print       $query->end_fieldset().
            $query->end_div();
};
push @errors, $@ if length $@;


# status fieldset

eval {
    if ($api->isClosing() || $api->isOpening() || $api->isBlocked()) {
        print
            $query->start_div().
                $query->start_fieldset({ class => 'fieldset-auto-width' }).$query->legend('Status');
        if ($api->isStoppedInTheMiddle()) {
            print   $query->p('The door is partially opened with the motor stopped.');
        } else {
            print   $query->p('The door is '.$api->getDoorStatus().'.');
        }
        print   $query->end_fieldset().
            $query->end_div();
    }
};
push @errors, $@ if length $@;


# errors fieldset

if (@errors) {
        print
            $query->start_div().
                $query->start_fieldset({ class => 'fieldset-auto-width error' }).$query->legend('Errors');
        print       $query->p($_).$query->br() for @errors;
        print   $query->end_fieldset().
            $query->end_div();
}


# closing HTML tags

print   $query->end_form().
    $query->end_center().
    $query->end_html();
