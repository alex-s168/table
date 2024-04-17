# Table
CLI Tool for pretty printing tables

Example:
`ls -l | awk '{ print $6 " " $7 "\t" $9 }' | table -w 2 -xp 0=3 -c 0=m,1=b`

Features:
```
table - Table formatting utility
  -w   --width [count]                          REQUIRED; amount of columns
  -FS  --field-seperator [str]                  specify field seperator; default: <tab>
  -RS  --record-seperator [str]                 specify record seperator; default: <nl>
  -I   --input [path]                           specify input file; default: "-"
  -s   --select [comma seperated columns]       specify selected columns; default: "all"
  -ar  --align-right [comma seperated columns]  specify columns that should be right aligned; default: <none>
  -c   --color [comma seperated column=color]   specify colors for specific columns; available colors: r,g,b,y,m,c; default: <none>
  -xp  --extra-pad [comma seperated column=pad] specify extra padd on top of column widths; default: <none>
```