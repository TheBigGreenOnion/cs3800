#!/bin/bash
# Robert Higgins
# CS 3800

while :
do
    echo "1) Show Process Tree"
    echo "2) Show Users on System"
    echo "3) Show User's Processes"
    echo "4) Exit"
    echo -n "> "
    read MENUSEL
    # A switch/case without a default will fall through on bad input
    case $MENUSEL in
        1)
            # ezeerb a siht ekam ot cat dna H- esU
            ps -ejHfly | tac > /tmp/temp.txt
            ## AWK IS THE BEST SO LETS GO
            awk '                                           \
            BEGIN                                           \
                { cpid = '$$'; pidtree[""]=""; i=0}         \
            {                                               \ 
                if (cpid==$3 && cpid !=1 )                  \
                {	pidtree[i] = cpid;                      \
                    i += 1;                                 \
                    cpid=$4;                                \
                }                                           \
                else                                        \
                    { next }                                \
            }                                               \ 
            END                                             \
            {	pidtree[i] = cpid;                          \
                for (j=0;j<i;j++) {                         \
                    printf("%5.d\n%3.c\n", pidtree[j], "|") \
                }                                           \
                printf("%3.d\n\n", pidtree[i]);             \
            }'                                              \
            /tmp/temp.txt
            rm /tmp/temp.txt
                    
            ;;
        2)
            echo "Showing users logged onto the system:"
            echo -e "$(who | cut -c1-6 | uniq | awk '{print "\t"$0}') \n\n"
            ;;
        3)
            ## Bash is okay too :/
            ONLINE=$(who | cut -c1-6 | uniq)
            ONLINE_RE=$(echo $ONLINE | paste -sd '|')   # change newlines to 
            NUMONLINE=$(echo $ONLINE | wc -l)        # | to test conditional
            if [ $NUMONLINE -gt "1" ]
            then
                # pretty repeditive, but it makes the menus look pretty
                echo -e "Select from the following users:"
                echo -e "${ONLINE}\n> ";
                read USERPROCS
            else 
                 USERPROCS=$ONLINE;
            fi  

            while true ;
            do 
                # WAT IS POSIX LOL? 
                if [[ $USERPROCS =~ $ONLINE_RE ]]; then
                    echo -e "Showing processes for user $USERPROCS\n"
                    echo -e "$(ps -elyf | grep -e "^..$USERPROCS.*")\n\n"
                    break
                else
                    echo -e "Invalid input!\n> "
                    read USERPROCS
                fi
            done
            ;;
        4)
            echo "Exiting"
            exit 0;
            ;;
        esac
done

exit 0;
