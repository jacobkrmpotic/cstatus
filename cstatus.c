#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

// OPTS
int delay = 3; // recount delay
char *xsetr;   // output string
char c_hour = 24;

// TIME ///////////////////////////////////////////////////////////////////////
int hour;

// add time to buffer
void p_time(char *st) {
    time_t sec = time(NULL);
    struct tm t = *localtime(&sec);
    char *time_st = malloc(20);

    // adjust for 12 hour clock
    int hour = t.tm_hour;
    if (c_hour == 12 && hour > 12) {
        hour -= 12;
    }

    // adjust for single digit minutes
    if (t.tm_min < 10) {
        snprintf(time_st, 20, "%d:0%d", hour, t.tm_min);
    } else {
        snprintf(time_st, 20, "%d:%d", hour, t.tm_min);
    }
    strncat(st, time_st, strlen(time_st)+1);

    // if clock is 12h, add AM/PM
    if (c_hour == 12) {
        if (t.tm_hour < 12) {
            strncat(st, "AM ", 3);
        } else {
            strncat(st, "PM ", 3);
        }
    }
}

// DATE //////////////////////
void p_date(char *st) {
    time_t sec = time(NULL);
    struct tm t = *localtime(&sec);
    char *date_st = malloc(20);

    snprintf(date_st, 20, "%d/%d/%d ", t.tm_year+1900, t.tm_mon, t.tm_mday);
    strncat(st, date_st, strlen(date_st)+1);
}

// BATTERY ////////////////////////////////////////////////////////////////////
// THINKPAD T420 ARCH LINUX BATTERY STATUS FILES
// IF USING ON A DIFFERENT COMPUTER, THESE ARE LIKELY
// NOT THE SAME
char *file_bat_type = "/sys/devices/LNXSYSTM:00/LNXSYBUS:00/PNP0A08:00/device:08/PNP0C09:00/PNP0C0A:00/power_supply/BAT0/status";
char *file_bat_current = "/sys/devices/LNXSYSTM:00/LNXSYBUS:00/PNP0A08:00/device:08/PNP0C09:00/PNP0C0A:00/power_supply/BAT0/energy_now";
char *file_bat_full = "/sys/devices/LNXSYSTM:00/LNXSYBUS:00/PNP0A08:00/device:08/PNP0C09:00/PNP0C0A:00/power_supply/BAT0/energy_full";
//char *file_bat_mode = "/sys/devices/LNXSYSTM:00/LNXSYBUS:00/PNP0A08:00/device:08/PNP0C09:00/PNP0C0A:00/power_supply/BAT0/type";

long int bat_current; // integer of energy level
float bat_percent; // battery level
char *bat_type; // battery type string

// read a long integer from a file
long int get_bat_l(char *file) {
    FILE *f = fopen(file, "r");
    char text[sizeof(f)];
    fgets(text, sizeof(f), f); 
    fclose(f);
    return atol(text);
}

// read the first string of a file (first 16 chars)
void get_bat_st(char *st, char *file) {
    // read file
    FILE *f = fopen(file, "r");
    // strcpy(st, f);
    fgets(st, 16, f);
    fclose(f);
}

void p_bat_percent(char *xsr) {
    // read current and max powerlevels, then calc percent
    bat_current = get_bat_l(file_bat_current);
    bat_percent = 100*( (float)bat_current / (float)get_bat_l(file_bat_full) );
    // max percent is 100 (duh)
    if (bat_percent > 100.000) {
        bat_percent = 100.0;
    }
    
    char *temp = malloc(8);
    snprintf(temp, 8, "%.2f", bat_percent);
    strncat(xsr, temp, strlen(temp));
    strncat(xsr, "% ", 2);
}

// add battery type to output buffer
void p_bat_type(char *xsr) {
    // read battery type (battery/nobat)
    bat_type = malloc(30); // max length of 30 bytes
    get_bat_st(bat_type, file_bat_type); 
    // add to buffer
    // strncat(xsr, bat_type, strlen(bat_type)-1);
    if (bat_type[0] == 'D') {
        strncat(xsr, "B", 1);
    } else if (bat_type[0] == 'C') {
        strncat(xsr, "C", 1);
    } else {
        strncat(xsr, "F", 1);
    }
    strcat(xsr, " ");
}

// MAIN LOOP //////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {
    // enabled
    char en_bat = 0;
    char en_time = 0;
    char en_date = 0;
    char en_vol = 0;
    int z = 1;
    while (z < argc) {
        // print help
        if (strcmp(*(argv+z), "-h") == 0) {
            printf("cstatus basic usage\n");
            printf("\033[3;39;49m\t$ cstatus -<opts> &\033[0;39;49m\n");
            printf("Where b = battery, t = time, d = date, v = volume.\n");
            printf("Options: \n");
            printf("Change the default delay from 3 (in seconds).\n");
            printf("\t\033[3;39;49m$ cstatus -b --delay 5\033[0;39;49m\n");
            printf("Use a 12 hour clock, as opposed to 24 (default).\n");
            printf("\t\033[3;39;49m$ cstatus -t -H12\033[0;39;49m\n");

            return 0;
        // enable battery display
        } else if (strcmp(*(argv+z),"-b") == 0) {
            en_bat = 1;
        // enable time display
        } else if (strcmp(*(argv+z), "-t") == 0) {
            en_time = 1;
        // enable date display
        } else if (strcmp(*(argv+z), "-d") == 0) {
            en_date = 1;
        // enable volume display
        } else if (strcmp(*(argv+z), "-v") == 0) {
            en_vol = 1;
        // set 24 hour mode
        } else if (strcmp(*(argv+z), "-H12") == 0) {
            c_hour = 12;
        // set delay (seconds)
        } else if (strcmp(*(argv+z), "-D") == 0 ||
                strcmp(*(argv+z), "--delay") == 0) {
            if (z+1 < argc) {
                ++z;
            } else {
                printf("No time (in seconds) specified for delay parameter.");
                return 5;
            }
            delay = atoi(*(argv+z));
            if (delay < 0) {
                printf("Invalid delay specified, using default delay of 3.");
                delay = 3;
            }
        } else {
            printf("Unrecognized option, '%s'.\n", argv[z]);
            return 1;
        }
        ++z;
    }

    char running = 1;
    while (running == 1) {
        // output buffer
        xsetr = calloc(256, 1);
        strncat(xsetr, "xsetroot -name \" ", 17);
        // bat
        if (en_bat == 1) {
            // battery
            p_bat_percent(xsetr);
            p_bat_type(xsetr);
        }
        if (en_vol == 1) {
            // add volume
            return 1;
        }
        if (en_date == 1) {
            p_date(xsetr);
        }
        if (en_time == 1) {
            p_time(xsetr);
        }
        strncat(xsetr, "\"\0", 2);
        if (strlen(xsetr) > 16) {
            system(xsetr);
        }
        printf("%s\n", xsetr);
        sleep(delay);
    }
}
