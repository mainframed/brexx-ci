/* ---------------------------------------------------------------------
 * Return Time in hundreds of seconds since Midnight
 * ............................... Created by PeterJ on 30. January 2020
 * ---------------------------------------------------------------------
 */
stime:
PARSE VALUE TIME('L') WITH __HH':'__MM':'__SS'.'__HS
return __HH*360000+__MM*6000+__SS*100+__HS
