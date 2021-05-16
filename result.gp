set title "File Copy"
set xlabel "Buffer size"
set ylabel "Time in sec"
set terminal png font "Times_New_Roman,12"
set output "file_copy_analysis.png"
set xtics (32, 64, 128, 256, 512)

plot \
"raw/read_write" using 1:2 with linespoints linewidth 2 title "read/write", \
"raw/mmap_write" using 1:2 with linespoints linewidth 2 title "mmap/write", \
"raw/sendfile" using 1:2 with linespoints linewidth 2 title "sendfile"