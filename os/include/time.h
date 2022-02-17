/* 时间 */

#ifndef __TIME_H__
#define __TIME_H__

struct time {
	/* 记录时间 */
	int second;
	int minute;
	int hour;
	int day;
	int month;
	int year;
};

struct time time;

#define	BCD2BIN(value)	(((value) & 0xf) + ((value) >> 4 ) * 10)
int get_cmos_time(struct time *time);

#endif
