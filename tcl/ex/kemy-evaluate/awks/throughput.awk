#Author:        ETAF
#input:         queue trace file
#output:        <time,througput> arrs

BEGIN{
    cnt = 0
    start_time = -1
    pkt_sum[0] = 0
}
{
    event = $1
    curr_time = $2
    pkt_size = $6
    
    #receive
    if(event == "r"){
        if(start_time == -1){
            start_time = curr_time
        }
        pkt_sum[cnt+1] =pkt_sum[cnt] + pkt_size
        times[cnt] = curr_time
        cnt++
    }
}
END{
    througput = 0
    for(i=1;i<cnt;++i){
        througput = (float)pkt_sum[i] /( times[i]-start_time) * 8 /1024/1024
        printf("%f %f\n",times[i],througput)
    }
    #printf("%fMbps\n", througput)
}
