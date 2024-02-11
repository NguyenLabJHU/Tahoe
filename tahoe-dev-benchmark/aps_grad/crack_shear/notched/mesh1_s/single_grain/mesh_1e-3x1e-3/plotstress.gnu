#set term post color
set term post eps color 22
#set title "STRESS STRAIN curves"
set output "force.eps"
#set xrange [0:.1]
#set yrange [0:1.4e5]
#set key .4,1e5
#
#set xlabel "TIME (sec)"
set xlabel "DISPLACEMENT (m)"
set ylabel "REACTION (N)"
#plot 'R.txt' using 1:2  t "no curl" with lines 1, 'R_curl.txt' using 1:2  t "with curl" with lines 2, 'R_curl_zero.txt' using 1:2  t "with curl zero" with lines 3, 'R_zero.txt' using 1:2  t "with curl zero" with lines 4
plot 'R_nocurl.txt' using 1:2  t "no curl" with lines 1, 'R_curl.txt' using 1:2  t "with curl" with lines 2
pause -1 "Click on this window and hit return when you are done"
#
