#include "utils.h"
#include <string.h>
#include <stdio.h>

void get_current_timestamp(char* tss){
    time_t timep;
	time(&timep);
	struct tm *p;
	p = gmtime(&timep);
    sprintf(tss,"%d/%d/%d-%d:%d:%d",p->tm_year - 100,1+p->tm_mon,p->tm_mday,8+p->tm_hour,p->tm_min,p->tm_sec);
}
