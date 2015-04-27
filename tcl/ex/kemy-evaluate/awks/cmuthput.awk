#!/usr/bin/awk -f
#===============================================================================
#
#          File:  cmuthput.awk
# 
#   Description:  
# 
#   VIM Version:  7.0+
#        Author:  YOUR NAME (), 
#  Organization:  
#       Version:  1.0
#       Created:  Thursday, April 23, 2015 20:54
#      Revision:  ---
#       License:  
#===============================================================================
{

    event = $1
    time = $2
    pkt_size = $6

    if( event == "r")
    {
        recv_sum += pkt_size
        printf("%f %f\n", time, recv_sum)
    }
}
