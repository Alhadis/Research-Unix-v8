s/Jan/.nr OM 0\n/
s/Feb/.nr OM 31\n/
s/Mar/.nr OM 59\n/
s/Apr/.nr OM 90\n/
s/May/.nr OM 120\n/
s/Jun/.nr OM 151\n/
s/Jul/.nr OM 181\n/
s/Aug/.nr OM 212\n/
s/Sep/.nr OM 243\n/
s/Oct/.nr OM 273\n/
s/Nov/.nr OM 304\n/
s/Dec/.nr OM 334\n/
s/ \([0-9]\)%/.nr YD \1\n/
s/ \([0-9][0-9]\)%/.nr YD \1\n/
s/\([0-9]*\):\([0-9]*\)/.nr HR \1\n.nr MI \2/
