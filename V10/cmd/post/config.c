#include "switch.h"
#include "config.h"
modsw_t modsw[] = {  
 "pq", pq_open, pq_close, pq_read, pq_write, 
 "join", join_open, join_close, join_read, join_write, 
 "opt", opt_open, opt_close, opt_read, opt_write, 
 "call", call_open, call_close, call_read, call_write, 
 "sys", sys_open, sys_close, sys_read, sys_write, 
 "ipc", ipc_open, sys_close, sys_read, sys_write, 
 0 };
