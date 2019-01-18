# These signal are necessary for uC++, so tell gdb to ignore them
# and pass to through to the program.
handle SIGALRM nostop noprint pass
handle SIGUSR1 nostop noprint pass
# Load macros to make gdb understand uC++ user-threads
source /usr/local/u++-7.0.0/gdb/utils-gdb.gdb
source /usr/local/u++-7.0.0/gdb/utils-gdb.py
# Have gdb indent complex values to make them readable.
set print pretty
