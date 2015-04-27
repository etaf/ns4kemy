#!/usr/bin/awk -f
#===============================================================================
#
#          File:  rtt_delay.awk
# 
#   Description:  
# 
#   VIM Version:  7.0+
#        Author:  YOUR NAME (), 
#  Organization:  
#       Version:  1.0
#       Created:  Thursday, April 23, 2015 22:13
#      Revision:  ---
#       License:  
#===============================================================================
BEGIN{
    rtts[0] = 10
    rtts[1] = 50
    rtts[2] = 100
    rtts[3] = 120
    rtts[4] = 150
    rtts[5] = 180
    rtts[6] = 200
    rtts[7] = 250

    M = iter * senders
}

{
    sum += $5
    if(NR%M == 0) {
        printf("%f\n",sum/M * 1000 - rtts[ NR/M - 1]);
        sum = 0;
    }


}

