#!/bin/bash
grep WRITER_TCPLOG /usr/local/bro/spool/logger/std*.log | sed 's/.*log://g' | sed 's/\/.*range /,/g' >logger_stderr.log
for size in `cat logger_stderr.log | tr ',' ' ' | awk '{print $2}' | sort -u` ; 
do 
    echo Size $size count
    echo $(for type in `cat logger_stderr.log | sed 's/,.*//g' | sort -u` ; 
    do 
        grep $type logger_stderr.log | grep $size | tail -1 | tr ',' ' ' | awk '{print $3"+"}'; 
    done) 0 | bc; 
done
for size in `cat logger_stderr.log | tr ',' ' ' | awk '{print $2}' | sort -u` ; 
do 
    echo Size $size limit $(grep $size logger_stderr.log | tr ',' ' ' | awk '{print $5}' | sort -u | tail -1); 
done
