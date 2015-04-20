{
    event = $1
    curr_time = $2
    pkt_id = $12
    if( event == "+" && send_times[pkt_id] == 0){
        send_times[pkt_id] = curr_time
    }
    if( event == "r"){
        printf("%f %f\n", curr_time, (curr_time - send_times[pkt_id])*1000 - 100)
    }
}


