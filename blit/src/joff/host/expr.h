enum exprtag {
	E_SYM,
	E_ID,
	E_CONST,
	E_UNARY,
	E_BINARY,
};

struct expr {
	enum exprtag	e_tag;
	char		*e_id;
	char		e_op;
	char		e_hostr;		/* structure constant in host */
	char		e_reg;
	struct 	expr	*e_left, *e_right;
	MLONG		e_lvalue;
	long		e_rvalue;
	long		e_mcc;
	struct ramnl	*e_tyid;
	struct ramnl	*e_sym;
	struct dim	*e_dim;
};

struct dim {
	int		d_dim;
	struct dim	*d_next;
};

struct expr *enode();
struct dim *dnode();
