/* sensor_filter.c -- ORIGINAL version submitted for review.
 * Moving-average filter for a 12-bit ADC current sensor.
 * Contains deliberate defects; see CODE_REVIEW.md. Do not use. */
#include <stdio.h>

#define BUF_SIZE 8

static int buffer[BUF_SIZE];
static int head;
static int count;

void filter_push(int value)
{
    buffer[head] = value;
    head++;
    if (head > BUF_SIZE)            /* D1 */
        head = 0;
    if (count < BUF_SIZE)
        count++;
}

int filter_average(void)
{
    int sum;                        /* D2 */
    int i;
    for (i = 0; i <= count; i++)    /* D3 */
        sum += buffer[i];
    return sum / count;             /* D4 */
}

unsigned char to_percent(int raw)   /* raw: 0..4095 */
{
    return raw * 100 / 4095;        /* D5 */
}

int over_current(int avg)
{
    int limit = 3500;
    if (avg = limit)                /* D6 */
        return 1;
    return 0;
}

const char *level_name(int pct)
{
    char name[16];                  /* D7 */
    switch (pct / 25) {             /* D8 */
    case 0: sprintf(name, "low"); break;
    case 1: sprintf(name, "medium"); break;
    case 2: sprintf(name, "high");
    case 3: sprintf(name, "critical"); break;
    }
    return name;
}
