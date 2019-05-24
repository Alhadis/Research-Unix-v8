define(`define_counter', `define($1,$2)')
define(`define_array', `changequote`'define($1,$2\n$`'1)`'changequote(Q++,Q--)')
define(`define_string', `define($1,$2)')
define(`unequal_string_registers', `"!@\*($1@\*($2@"')
define(`equal_string', `"@\*($1@$2@"')
define(`unequal_string', `"!@\*($1@$2@"')
define(`null_string', `"@\*($1@@"')
define(`not_null_string', `"!@\*($1@@"')
define(`null_strings', `"@\*($1\*($2\*($3\*($4@@"')
define(`not_null_strings', `"!@\*($1\*($2\*($3\*($4@@"')
define(`gt',`changequote`'ifelse(len($1),1,"\n$1>$2","\n`('$1>$2")`'changequote(Q++,Q--)')
define(`ge',`changequote`'ifelse(len($1),1,"\n$1>=$2","\n`('$1>$2")`'changequote(Q++,Q--)')
define(`eq',`changequote`'ifelse(len($1),1,"\n$1=$2","\n`('$1>$2")`'changequote(Q++,Q--)')
define(`le',`changequote`'ifelse(len($1),1,"\n$1<=$2","\n`('$1>$2")`'changequote(Q++,Q--)')
define(`macro_argument_eq', `"\$$1=$2"')
changecom
undefine(`undefine', `defn', `pushdef', `popdef', `ifdef', `shift',
	`changecom', `divert', `undivert', `divnum', `dnl', `incr',
	`decr', `eval', `index', `substr', `translit', `sinclude',
	`syscmd', `sysval', `maketemp', `m4exit', `m4wrap', `errprint', `dumpdef',
	`traceon', `traceoff')
changequote(`Q++',`Q--')
