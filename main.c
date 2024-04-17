#include <stdio.h>

#include "minilibs/utils.h"
#define MINITAB_IMPL
#include "minilibs/minitab.h"
#define CLI_IMPL
#include "minilibs/cli.h"
#define FILELIB_IMPL
#include "minilibs/filelib.h"

static const char *flag(const int argc, char **argv, const char *name1, const char *name2, const char *defaul) {
    Flag flag = getFlag(argc, argv, name1);
    if (!flagExist(flag))
        flag = getFlag(argc, argv, name2);
    const char *str = defaul;
    if (flag.str != NULL)
        str = flag.str[1];
    return str;
}

struct IntList {
    int *items;
    size_t len;
};

static struct IntList parseIntList(char *list, const size_t allLen) {
    if (strcmp(list, "all") == 0) {
        struct IntList res;
        res.len = allLen;
        res.items = malloc(sizeof(int) * allLen);
        for (size_t i = 0; i < allLen; i ++) {
            res.items[i] = i;
        }
        return res;
    }

    struct IntList res;
    res.len = 0;
    res.items = NULL;

    SPLITERATE(list, ",", p) {
        char *end;
        const int col = strtol(p, &end, 10);
        if (*end) {
            continue;
        }
        res.items = realloc(res.items, sizeof(int) * (res.len + 1));
        res.items[res.len ++] = col;
    }

    return res;
}

int main(const int argc, char **argv) {
    if (flagExist(getFlag(argc, argv, "--help")) ||
        flagExist(getFlag(argc, argv, "-h"))) {
        printf("\"%s\" - Table formatting utility\n", argv[0]);
        printf("  -w   --width [count]                          REQUIRED; amount of columns\n");
        printf("  -FS  --field-seperator [str]                  specify field seperator; default: <tab>\n");
        printf("  -RS  --record-seperator [str]                 specify record seperator; default: <nl>\n");
        printf("  -I   --input [path]                           specify input file; default: \"-\"\n");
        printf("  -s   --select [comma seperated columns]       specify selected columns; default: \"all\"\n");
        printf("  -ar  --align-right [comma seperated columns]  specify columns that should be right aligned; default: <none>\n");
        printf("  -c   --color [comma seperated column=color]   specify colors for specific columns; available colors: r,g,b,y,m,c; default: <none>\n");
        printf("  -xp  --extra-pad [comma seperated column=pad] specify extra padd on top of column widths; default: <none>\n");
        fputc('\n', stdout);
        return 0;
    }

    const char *wStr = flag(argc, argv, "--width", "-w", NULL);
    if (wStr == NULL) {
        fprintf(stderr, "width not specified!\n");
        return 1;
    }

    char *end;
    const size_t w = strtol(wStr, &end, 10);
    if (*end) {
        fprintf(stderr, "Invalid width!\n");
        return 1;
    }
    
    const char *fieldSep = flag(argc, argv, "--field-seperator", "-FS", "\t");
    const char *recordSep = flag(argc, argv, "--record-seperator", "-RS", "\n");
    if (recordSep[1] != '\0') {
        fprintf(stderr, "Only single-char record seperators supported!\n");
        return 1;
    }
    const char *input = flag(argc, argv, "--input", "-I", "-");
    char *selectIn = strdup(flag(argc, argv, "--select", "-s", "all"));
    struct IntList select = parseIntList(selectIn, w);
    char *alignRightIn = strdup(flag(argc, argv, "--align-right", "-ar", ""));
    const struct IntList alignRight = parseIntList(alignRightIn, w);
    char *color = strdup(flag(argc, argv, "--color", "-c", ""));
    char *extraPad = strdup(flag(argc, argv, "--extra-pad", "-xp", ""));

    Table table;
    table.cols = w;
    table.rows = 0;
    table_create(&table);

    for (size_t i = 0; i < alignRight.len; i ++) {
        const size_t col = alignRight.items[i];
        if (col >= w)
            continue;
        table.colsPadLeft[col] = true;
    }

    SPLITERATE(extraPad, ",", p) {
        size_t col;
        size_t extra;
        if (sscanf(p, "%zu=%zu", &col, &extra) != 2 || col >= table.cols) {
            fprintf(stderr, "Invalid column=extrapad \"%s\"!\n", p);
            return 1;
        }
        table.colsExtraPad[col] = extra;
    }

    FILE *in;
    if (strcmp(input, "-") == 0)
        in = stdin;
    else {
        in = fopen(input, "r");
        if (in == NULL) {
            fprintf(stderr, "File not found!\n");
            return 1;
        }
    }

    char *record;
    while ((record = readSplitting(in, recordSep[0])) != NULL) {
        table_append_row(&table);
        size_t x = 0;
        size_t realx = 0;
        SPLITERATE(record, fieldSep, p) {
            bool sel = false;
            for (size_t i = 0; i < select.len; i ++) {
                if (select.items[i] == realx) {
                    sel = true;
                    break;
                }
            }
        
            realx ++;
        
            if (sel) {
                char *copy = strdup(p);
                table_cell(table, x, table.rows-1).text = copy;
                table_cell(table, x, table.rows-1).toFree = copy;

                x ++;
            }
            
            if (x == table.cols)
                break;
        }

        free(record);
    }

    fclose(in);
    
    SPLITERATE(color, ",", p) {
        size_t col;
        static char colorstr[256];
        if (sscanf(p, "%zu=%s", &col, &colorstr) != 2 || col >= table.cols) {
            fprintf(stderr, "Invalid column=color \"%s\"!\n", p);
            return 1;
        }
        if (colorstr[1] != '\0') {
            fprintf(stderr, "Invalid color \"%s\"!\n", colorstr);
            return 1;
        }
        const char *rcolor;
        switch (colorstr[0]) {
            case 'r':
                rcolor = ANSI_COLOR_RED;
                break;
            
            case 'g':
                rcolor = ANSI_COLOR_GREEN;
                break;

            case 'b':
                rcolor = ANSI_COLOR_BLUE;
                break;

            case 'y':
                rcolor = ANSI_COLOR_YELLOW;
                break;
            
            case 'm':
                rcolor = ANSI_COLOR_MAGENTA;
                break;

            case 'c':
                rcolor = ANSI_COLOR_CYAN;
                break;
            
            default:
                fprintf(stderr, "Invalid color \"%s\"!\n", colorstr);
                return 1;
        }
        table_color_col(table, rcolor, col);
    }

    table_print(table, stdout);
    
    return 0;
}
