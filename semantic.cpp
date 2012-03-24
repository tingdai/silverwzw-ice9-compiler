extern "C" {
#include "ast.h"
#include "ice9.tab.h"
#include "parse.h"
}

int main() {
	parse();
	return 0;
}
