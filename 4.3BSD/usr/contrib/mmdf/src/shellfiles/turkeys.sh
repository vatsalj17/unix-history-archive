checkmsgs | sed -e "/Message msg/d" -e "/Return address/d" \
	-e 's/ (via .*)$//' -e 's/^.*@//' | sort | uniq -c | sort -n -r
