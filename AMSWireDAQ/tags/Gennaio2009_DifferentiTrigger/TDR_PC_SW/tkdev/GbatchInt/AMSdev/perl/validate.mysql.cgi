#!/usr/bin/perl -w
#  $Id: validate.mysql.cgi,v 1.1.1.1 2007/11/13 09:56:12 zuccon Exp $
use Gtk;
use strict;


use lib::RemoteClient;


my $debug="-d";

unshift @ARGV, "-Dmysql";
unshift @ARGV, "-F:AMSMC02:pcamsf0";
unshift @ARGV, "-N/cgi-bin/mon/validate.mysql.cgi";




my $nocgi=1;
my $html=new RemoteClient($nocgi);


$html->Validate();






