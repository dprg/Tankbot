/* ------------------------------------------------------------ */
/* pid.c		Speed controller for jbot

21 Jan 05 dpa	Created fron srat04/src/speedctl.c
21 Feb 05 dpa	Inverted left/right to match encoders (?)
		in motorcmd()
01 May 05 dpa	g->pwm_enable 1 = left only, 2 = right only, 3 = both.
08 May 05 dpa	Don't PID if radio_enable == 2 and radio_good == 3;
		(Hard to drive manually with PID turned on).
		g->radio_enable = 1, use PID if enabled
		g->radio_enable = 2, don't use PID even if enabled,
		while radio_good == 3.

/* ------------------------------------------------------------ */

#include "param.h"
#include "rc.h"

/* ------------------------------------------------------------ */

#define AMAX 1150000
#define TMAX 11500
#define TICKSPERCENT (TMAX/100)


/* ------------------------------------------------------------ */
/* global static */

/*
int g->pid_enable;
int g->kpro, g->kdrv, g->kint;
*/

/* local */

int lpro, rpro, ldrv, rdrv, ltint, rtint;
int last_lpro, last_rpro;


/* ------------------------------------------------------------ */


int speedctrl(int Lvel, int Rvel)
{
        int extern left_velocity,right_velocity;

        if (g->pid_enable) { 

                lpro = ((Lvel*TICKSPERCENT) - left_velocity);
                rpro = ((Rvel*TICKSPERCENT) - right_velocity);

		ldrv = lpro - last_lpro;
		last_lpro = lpro;

		rdrv = rpro - last_rpro;
		last_rpro = rpro;

		if (g->kpro) ltint += (lpro/g->kpro);
		if (g->kdrv) ltint += (ldrv/g->kdrv);

		if (ltint > AMAX) ltint = AMAX;
		else if (ltint < -AMAX) ltint = -AMAX;

		if (g->kpro) rtint += (rpro/g->kpro);
		if (g->kdrv) rtint += (rdrv/g->kdrv);
		if (rtint > AMAX) rtint = AMAX;
		else if (rtint < -AMAX) rtint = -AMAX;

                pwmL(ltint/g->kint); 
                pwmR(rtint/g->kint); 

        } 
}


/* ------------------------------------------------------------ */

int motorcmd(int translation, int rotation)
{
        extern int bot_speed, radio_good;
        int left, right, pid;

	pid = g->pid_enable;
	if ((g->radio_enable == 2) && (radio_good == RADIO_GOOD)) pid = 0;

        bot_speed = translation;

        left = translation + rotation;
        if (left > 100) left = 100;
        else if (left < -100) left = -100;

        right = translation - rotation;
        if (right > 100) right = 100;
        else if (right < -100) right = -100;

        if (pid) {
                speedctrl(left,right);
        } else {
		if (g->pwm_enable &1) {
                	pwmL(left);
		}
		if (g->pwm_enable &2) {
                	pwmR(right);
		}
        }
        return 0;
}


/* ------------------------------------------------------------ */
/* EOF */
