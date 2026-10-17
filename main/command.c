#include "command.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>
static bool number(const char *s, unsigned min, unsigned max, int32_t *value)
{
    if (!s || !*s) return false;
    for (const char *p=s;*p;++p) if (!isdigit((unsigned char)*p)) return false;
    errno=0; char *end;
    unsigned long n=strtoul(s,&end,10);
    if (errno || *end || n<min || n>max) return false;
    *value=(int32_t)n; return true;
}
static bool percent(const char *s, float *value)
{
    if (!s || !*s) return false;
    bool dot=false, digit=false;
    for (const char *p=s;*p;++p) {
        if (*p=='.' && !dot) dot=true;
        else if (isdigit((unsigned char)*p)) digit=true;
        else return false;
    }
    if (!digit) return false;
    char *end; errno=0;
    float f=strtof(s,&end);
    if (errno || *end || !isfinite(f) || f<0.1f || f>20) return false;
    *value=f; return true;
}
static bool eq(const char *a,const char *b) { return a && strcmp(a,b)==0; }
bool command_parse(const char *line, const tester_config_t *config, command_t *o)
{
    if (!line || !config || strlen(line)>=CLI_LINE_SIZE) return false;
    char text[CLI_LINE_SIZE]; strcpy(text,line);
    for (char *p=text;*p;++p) *p=(char)tolower((unsigned char)*p);
    char *v[5]={0}, *save=NULL;
    int n=0;
    for (char *t=strtok_r(text," \t\r\n",&save);t;t=strtok_r(NULL," \t\r\n",&save)) {
        if (n==5) return false;
        v[n++]=t;
    }
    if (!n) return false;
    *o=(command_t){0}; o->config=*config;
    const struct {const char *s; command_kind_t kind;} singles[]={
        {"version",CMD_VERSION},{"status",CMD_STATUS},{"reset",CMD_RESET},
        {"home",CMD_HOME},{"track0",CMD_TRACK0},{"wp",CMD_WP},{"index",CMD_INDEX}};
    for (unsigned i=0;i<sizeof(singles)/sizeof(singles[0]);++i)
        if (eq(v[0],singles[i].s) && n==1) { o->kind=singles[i].kind; return true; }
    if (eq(v[0],"help") && n<=2) {
        o->kind=CMD_HELP;
        if (n==2) {
            const char *topics[]={"help","version","status","reset","select","motor","home","seek","step",
                "track0","wp","index","rpm","read","power","test","config"};
            bool known=false;
            for (unsigned i=0;i<sizeof(topics)/sizeof(topics[0]);++i) if (eq(v[1],topics[i])) known=true;
            if (!known) return false;
            strcpy(o->name,v[1]);
        }
        return true;
    }
    if (eq(v[0],"select") && n==2) {
        o->kind=CMD_SELECT;
        if (eq(v[1],"off")) { o->value=-1; return true; }
        return number(v[1],0,3,&o->value);
    }
    if (eq(v[0],"motor") && n==2 && (eq(v[1],"on")||eq(v[1],"off"))) {
        o->kind=CMD_MOTOR; o->flag=eq(v[1],"on"); return true;
    }
    if (eq(v[0],"seek") && n==2) { o->kind=CMD_SEEK; return number(v[1],0,config->max_track,&o->value); }
    if (eq(v[0],"step") && (n==2||n==3) && (eq(v[1],"in")||eq(v[1],"out"))) {
        o->kind=CMD_STEP; o->flag=eq(v[1],"in"); o->value=1;
        return n==2 || number(v[2],1,config->max_track,&o->value);
    }
    if (eq(v[0],"rpm") && n<=2) {
        o->kind=CMD_RPM; o->value=config->rpm_samples;
        return n==1 || number(v[1],1,100,&o->value);
    }
    if (eq(v[0],"read") && (n==2||n==3)) {
        if (eq(v[1],"activity")) {
            o->kind=CMD_READ_ACTIVITY; o->value=config->read_sample_ms;
            if (n==2) return true;
            // Seconds, with millisecond precision, positive and bounded.
            float seconds;
            char *end; errno=0;
            for (char *p=v[2];*p;++p) if (!isdigit((unsigned char)*p) && *p!='.') return false;
            seconds=strtof(v[2],&end);
            if (errno || *end || !isfinite(seconds) || seconds<0.001f || seconds>60) return false;
            o->value=(int32_t)lroundf(seconds*1000); return true;
        }
        if (eq(v[1],"timing")) {
            o->kind=CMD_READ_TIMING; o->value=1000;
            return n==2 || number(v[2],1,10000,&o->value);
        }
        return false;
    }
    if (eq(v[0],"power") && (n==1 || (n==2 && eq(v[1],"watch")))) {
        o->kind=n==1?CMD_POWER:CMD_POWER_WATCH; return true;
    }
    if (eq(v[0],"test")) {
        if (n==2 && eq(v[1],"basic")) { o->kind=CMD_TEST_BASIC; return true; }
        if (n==2 && eq(v[1],"seek")) { o->kind=CMD_TEST_SEEK; return true; }
        if (n==3 && eq(v[1],"exercise")) {
            o->kind=CMD_TEST_EXERCISE; return number(v[2],1,100000,&o->value);
        }
        return false;
    }
    if (eq(v[0],"config") && n>=2) {
        if (n==2 && eq(v[1],"show")) { o->kind=CMD_CONFIG_SHOW; return true; }
        if (n==2 && eq(v[1],"save")) { o->kind=CMD_CONFIG_SAVE; return true; }
        if (n==2 && eq(v[1],"defaults")) { o->kind=CMD_CONFIG_DEFAULTS; return true; }
        if (n!=3) return false;
        o->kind=CMD_CONFIG_SET;
        if (eq(v[1],"rpm_tolerance")) {
            if (!percent(v[2],&o->config.rpm_tolerance)) return false;
        } else {
            int32_t value;
            if (!number(v[2],0,100000,&value)) return false;
            if (eq(v[1],"step_time")) o->config.step_time=value;
            else if (eq(v[1],"max_track")) o->config.max_track=value;
            else if (eq(v[1],"home_limit")) o->config.home_limit=value;
            else if (eq(v[1],"rpm_samples")) o->config.rpm_samples=value;
            else if (eq(v[1],"drive_select")) o->config.drive_select=value;
            else if (eq(v[1],"motor_spinup_ms")) o->config.motor_spinup_ms=value;
            else if (eq(v[1],"read_sample_ms")) o->config.read_sample_ms=value;
            else return false;
        }
        return config_validate(&o->config);
    }
    return false;
}
