typedef struct {
	char	*name;
	char	*(*open)();
	int	(*close)();
	int	(*read)();
	int	(*write)();
} modsw_t;

extern modsw_t	modsw[];
