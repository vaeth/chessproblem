#!/usr/bin/env perl
use warnings;
use strict;
my @result = ();
while (<DATA>) {
  chomp();
  s{\s*\#.*}{};
  next if ($_ eq '');
  if (@result) {
    my $expected = $_;
    my @want = split /;\s*/;
    my $fail = '';
    for my $i (@result) {
      if (!grep { $i eq $_ } @want) {
        printf STDERR "Wrong result: $i found but expected only $expected\n";
        $fail = 1
      }
    }
    for my $i (@want) {
      if (!grep { $i eq $_ } @result) {
        printf STDERR "Wrong result: expected solution $i not found\n";
        $fail = 1
      }
    }
    exit 1 if($fail);
    @result = ();
    next
  }
  my $cmd = "./chessproblem -n0 $_";
  print "$cmd\n";
  open(my $fh, '-|', $cmd) or die;
  while (<$fh>) {
    chomp();
    s{^.*\:\s*}{};
    push(@result, $_)
  }
  close($fh);
  die unless(@result)
}
1
__END__

-P -m2 "Kc1" "Ka1"
No solution exists
-m3 "Kc1,Na2,c7" "Ka1"
c7-c8=Q;c7-c8=R

# Chess problems by Martin Väth <martin@mvath.de>
#1
-m2 "Ka1,Qd7,Bd1,Be3,d3,g3" "Kf6"
Bd1-h5;Bd1-b3
#2
-m2 "Ka1,Qd7,Bd1,Be3,g3" "Kf6"
Bd1-c2;Bd1-b3
#3
-m2 "Ka1,Rd7,Ne2,Ng3,Bc1,Bg4" "Kf6"
Bg4-f5
#4
-m2 "Kh1,Qf8,Re5,Nh5,Nc4,Bd3,c5,a6" "Kd7,Qd8,Rb8,Re7,Bd4,Bc8,Ne8,Ng7,d5,d6,c6,c7,b7,a7,f5,h6"
Qf8*g7
#5
-m3 "Ka2,Qc4,Bb3,Ne8,d6,e7" "Kc8,Qh1,Rh3,Rh4,Ba8,Bf6,Nb7,Nd1,c3,g2,g3,g4,a7,c7,d7,c6"
Qc4*c6
#6
-m2 "Ka1,Qa7,Qd2,Rb1,Bh3,Ba5,Ne4,Nf4,b2,b3" "Kc8,Qe6,Re8,Rb7,Nh4,b5,c6,d7"
Qd2*d7
#7
-m2 "Ka1,Qa2,Re1,Rf1,Bd3,Bb8,Ne7,c3" "Ke5,Qe4,Rd6,Rh4,Bh3,Ne6,Ne3,b7,d5"
Qa2*d5
#8
-m3 "Kh1,Qa3,Rb3,Rd1,Bh2,Bf3,Ng2,Nf8,g3,e2" "Ke5,Qb6,Rb8,Rd6,Bc7,Bc6,Nb7,Ng7,b5,c5,c4,d4,e3,f4,f6,g5"
Rb3*e3
#9
-m2 "Ka2,Qg4,Re6,Rc7,Bb3,Bh3,Ng2,Na4,c2,c3,b4,f3" "Kd5,Qd8,Qf2,Rc4,Bb6,Be2,a7,e5,g5,f4,h4,g3"
Qg4*g5
#10
-m3 "Kg4,Rc8,Rf2,Ba8,Bf3,c3,c4" "Ke5,Bc6,d6,d7"
Bf3-d5
#11
-m4 "Kg7,Qe4,Rg4,Rc5,Bh6,Nd3,Ne2,h3" "Kh5,Qe5,Rf5,Bg5,Bf3,Nd5,f6,h4"
Rg4*h4;Ne2-g3
#12
-m2 "Kc5,Qh5,Re8,Ra2,Bg2,Ne7" "Ke3,Qe4,d3,e5,f2,g7,h6"
Qh5*e5
#13
-m2 "Kh7,Qe7,Rg3,Bg5,Bg2,Nc6,d5,f4" "Kf5,Re4,Bf6,Nc5,Nd6,d7,g7,g4"
Qe7-e5
#14
-m2 "Ka6,Qf7,Rh3,Bh7,Nb2,e2,c3,b4,h5" "Ke5,Qg5,d6,d5,e4,b5"
Rh3-e3
#15
-m3 "Kb2,Qd4,Ra4,Nh4,e5,f4,d3" "Kh1,g2,h2,h3,f5,e6"
Qd4-a7
#16
-m2 "Ke3,Qg3,Rc7,e4,g5" "Ke6"
Ke3-f4
#17
-m2 "Kf3,Qg6,Nf5,e2,c4,h4" "Ke5,h5,e3"
Kf3-g3
#18
-m2 "Kd5,Qb3,Rf4,Rg4,c6,d6,e4,f5,h6" "Kf6,Qe1,Bd2,Ne8,c7,c3,e7,e3,g7,h7"
Kd5-d4
#19
-m2 "Kg4,Qd7,f3,c5,e4" "Kf6"
Kg4-h5
#20
-m2 "Kc3,Qe1,Rh7" "Ke6,Rf4,Nd6,Nf6,d5,e5,f5"
Qe1*e5
#21
-m2 "Ke8,Qe3,Be2,c5,g5,c3,g3" "Ke6,d5,e5,f5"
Qe3-e4
#22
-m2 "Kc3,Qg3,Bh8,f2" "Kb1,a3"
Qg3-g8
#23
-m4 "Kd8,Rg1,Rh5,Ba1,Bd5,a6,c3,c4,e6,f5,g3,h6" "Kb8,a2,a3,a7,c5,e7,f6,h7"
Rh5-g5
#24
-m3 "Ka3,Re2,Nd2" "Ka1,Bc2,b2"
Re2-e1
#25
-m2 "Kh6,Qc4,c7,e7" "Kg8,Re3,Bh3,c6,f6,f7"
Qc4-e6
#26
-m2 "Kd7,Qh4,Nh8,Bf4,g4,d4" "Kf6,Rg5,Nf7"
Qh4*g5
#27
-m3 "Kg1,Qh8,Bc6,g4,d6" "Kh6,g7,h7,g6,g5,g3"
Bc6-h1
#28
-m3 "Kf4,Qb8,Nd3,Nd5,a2,b3,b5,c4,g4,h4" "Kd4,c5,a3,b6"
Kf4-f5
#29
-m4 "Kb6,Ra2,c2" "Kb4,a4,c3,c4"
Ra2-a1
#30
-m3 "Kb6,Rc1,a2,d2,c5" "Kb4,a4,d3,c6,b7"
Rc1-c3
#31
-m2 "Kd2,Qf2,Bd7" "Ke4"
Kd2-c3
#32
-m2 "Kf8,Qc3,Bb5" "Kd8"
Kf8-g7
#33
-m2 "Kf6,Qc2,Bb5" "Kd8"
Bb5-e8
#34
-m2 "Kg5,Qd3,c7" "Ke5"
c7-c8=N
#35
-m3 "Kc1,Bf5,c7" "Ka1"
c7-c8=R
#36
-m3 "Kg5,Qd7,Rf1,e5" "Kg7,Bg8,f7"
Qd7-e8
#37
-m2 "Kc3,Qc2" "Ke1,e3"
Qc2-g2

# Noam D. Elkies, Harvard 2002
-s2 "Kc1,Qf5,Ne1" "Kh8,Bd1,a2,c2,c3,e2"
Qf5-f7
# Lord Dunsany
-m4 "Kd8,Qe8,Ra8,Rh8,Bc8,Bf8,Nb8,Ng8" "Ke1,Qd1,Ra1,Rh1,Bc1,Bf1,Nb1,Ng1,a2,b2,c2,d2,e2,f2,g2,h2"
Nb8-c6;Nb8-d7
# Friedrich Amelung 1897
-eg -m2 "Kf5,Rh8,e5,f6,h5" "Kh6,g5,h7"
h5*g6ep
# Martin Kahl 1930
-h3 "Kb8,Ra8,Nd1,a7,g2" "Kf1,Qc6"
Qc6*a8 Kb8-c7 Qa8*g2 a7-a8=Q Qg2-e2 Qa8-h1
# Stefan Schneider 1974
-s4 "Kc1,Ra8,Be5,Nd2,Nb4,h6" "Ka1,Bg8,Na5,Nd4,c2,d3,b5,d5,h7"
Be5-h8
# Tivadar Kardos 1964
# a
-m2 "Kc4,Rh7,f4,b6,c7" "Kd6"
c7-c8=B
# b
-m2 "Kc4,Rh7,f4,g4,c7" "Kd6"
c7-c8=R
# c
-m2 "Kc4,Rh7,f4,g4,c7" "Ka6"
c7-c8=N
# d
-m2 "Kc4,Rh7,f4,g4,c7" "Kf6"
c7-c8=Q
# Frank Healey 1861
-m3 "Kh2,Qg6,Rd1,Rf3,Ba1,Nb6,Nf7,a3,c3,d2,d5,g2" "Kc5,Bb5,Nb7,a4,c4,f4,g7"
Rd1-h1
# Dr. Ado Kraemer, Dr. Erich Zepler 1948
-m3 "Kf2,Qh6,Rd5,Bc5,Nc7,Nf6,b4,b6,d7" "Kc6,Ra8,Rf8,Nf5,c2,h2,a6,b7"
Kf2-e1
# Dr. Jan Dobrusky 1901
-m4 "Kb6,Qa1,Bd7,Nd5,Nf2" "Ke5,Bc4,Bf4,Nd4,d3,c5,d6"
Qa1-a8
# Samuel Loyd 1903
-m3 "Kf1,Ra5,Rf6,Bb5,Bg7,Nb6,Ne4,b4,d2" "Ke5,Re8,Rh2,Bg3,Bg8,Nh1,Na2,a6,b7,c3,e6,f2,h4"
Kf1-e2
# Samuel Loyd 1867
-P -m5 "Kh5,Rb5,Re2,Na1,Nh3,b2,c2,g3" "Kh1,Rc8,Bd8,Na8,a3,e3,b6,b7,f7,h7"
b2-b4
# Frederick E. Gamage 1946
-m3 "Kg2,Rb6,Rc7,Ba3,Be2,Na4,Ne4,b3,c2,d3,h2" "Kc1,Rb1,Rb2,a2,a5,b7,g7,h7"
Kg2-h1
# Samuel Loyd 1889
-m2 "Kf1,Qa4,Rb2,Rh1,Ba3,Bd3,Ng2,Nh3,e2,f3,g3,h4" "Kc1,Qh6,Rd8,Rh8,Bb7,Bg7,Ne1,a7,b6,c7,f7,h7"
Ba3-f8
# Dr. Ado Kraemer 1954
-m3 "Ke8,Rb1,Re2,Bd1,Nh3,d2,g4" "Kh1,f2,h2,h4"
Ke8-f8
# Frank Healey 1858
-m3 "Kd6,Qf2,Rc5,c3" "Kd3"
Kd6-d7
# Dr. Ado Kraemer 1943
-m3 "Kb3,Qa2,Rc7,Be5,Nd7" "Ka8,Bc3,Nb2,Na4,b4,b5"
Be5-h8
# Dieter Kutzborski 1971
-m3 "Kb2,Qb4,Be4,Bf6,Nd4,Nh4,h2,f3,e5" "Kf4,Qg8,Bf1,Bc7,Ne7,b3,e3,b5"
Kb2-a3
# Walter Grimshaw 1852
-m3 "Kg7,Rg1,Rd5,Bc6,Nh4,c2,f2,h3,h5" "Ke4,Qa6,Nb7,e2,c3,f4,b6,h6"
Rg1-f1
# Dr. Meindert Niemeijer 1919
-m2 "Kb3,Qc5,Rg1,Bf2,Ne3,c6" "Kf4,Rh3,Rf6,Bh6,e4,f3,h5,h7"
Kb3-a4
# Erich Brunner 1927
-m2 "Ka5,Qh1,Rf3,Re5,Bc5,Bh3,Nf7,Ne8,e2,d4,g5,g6,b7" "Kb8,h2,h4,g7"
Nf7-h8
# Erich Brunner 1910
-m3 "Ke5,Rc4,Rg1,Bh3,Na8,a4,a6,c3,c5,f6,g5" "Kc6,a5,c7,f7,g6"
Rc4-h4
# Erich Brunner 1926
-m3 "Ka7,Ra5,Rh5,Bb8,Ng8,Ne7,c3,g3,e4,e5,d6,f6" "Ke6,Rd3,Rf3,d7,f7,h6"
Ka7-a8
# Erich Brunner 1928
-m3 "Kb1,Qb5,Nb8,Nf5,d2,c4,f3,h4,g5,f6" "Ke6,Qd8,Bc8,b7,d4,f7"
Kb1-c1
# H. v. Gottschall 1892
-m2 "Kh7,Qb3,Be4,Nf7,g7" "Kf6,e6"
Be4-f5
# Samuel Loyd 1876
-m3 "Kh4,Re1,Bg3,Ng2,a7,b7" "Kh1,Bg1,Ba8"
b7*a8=N
# Dr. Ado Kraemer 1930
-m4 "Kg7,Ra5,Bc7,Bg8,Na4,b5,h7" "Kb4,Bg1"
h7-h8=B
# Sigurd Clausen 1939
-m3 "Kg8,Qf8,Rf2,g7" "Kg1,Re8"
Rf2-a2
