
BEGIN { INHIRES=0; INDEBUG=0; INSONY=0 }

END {
	if (INHIRES) print "*** ERROR: missing close HIRES tag ***";
	if (INDEBUG) print "*** ERROR: missing close DEBUG tag ***";
	if (INSONY) print "*** ERROR: missing close DEBUG tag ***";
}
/HIRES>>/ { INHIRES++; next }
/<<HIRES/ { INHIRES--; next }

/DEBUG>>/ { INDEBUG++; next }
/<<DEBUG/ { INDEBUG--; next }

/SONY>>/ { INSONY++; next }
/<<SONY/ { INSONY--; next }

# the complicated truth table:
# 		     INDEBUG / DEBUG
# INHIRES / HIRES    00 01 11 10
#		  00  1  1  1  0
#		  01  1  1  1  0
#		  11  1  1  1  0
#		  10  0  0  0  0
# extrapolates to this:
{ if (!((INDEBUG && !DEBUG) || (INHIRES && !HIRES) || (INSONY && !SONY))) { print $0;next } }
{ next }
