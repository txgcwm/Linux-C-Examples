#include  <stdlib.h>
#include  <strings.h>			// for strncasecmp
#include  <sys/stat.h>

#include  "IniFile.h"


int main(int argc, char **argv)
{
	char sz_value[256];
	int i_status = 0;
	long l_status = 0;
	double d_status = 0;
	int b_status;

	// 1. open ini file
	i_status = IniOpenFile("test.ini");
	if (i_status != 0)
		return -1;

	// 2. serach section
	b_status = IniSearchSection("sectionA");
	if (b_status == TRUE)
		DBGPRINT(("Main -- sectionA found\n"));
	else
		DBGPRINT(("Main -- sectionA not found.\n"));

	// 3. get/set value of string type
	memset(sz_value, 0, sizeof(sz_value));
	b_status = IniGetString("sectionA", "AID", sz_value);
	DBGPRINT(("Main -- sectionA--AID = %s\n", sz_value));

	DBGPRINT(("\nMain -- to Set String"));
	b_status = IniSetString("sectionC", "PID", "ABCD1234561234567890");
	DBGPRINT(("Main -- PID set to: %s\n", "ABCD1234561234567890"));

	memset(sz_value, 0, sizeof(sz_value));
	b_status = IniGetString("sectionC", "PID", sz_value);
	DBGPRINT(("Main -- sectionC--PID = %s\n", sz_value));

	memset(sz_value, 0, sizeof(sz_value));
	b_status = IniGetString("sectionC", "AID", sz_value);
	DBGPRINT(("Main -- sectionC--AID = %s\n", sz_value));

	// 4. get/set value of integer type
	i_status = IniGetInteger("sectionA", "NNN", 99);
	DBGPRINT(("Main -- sectionA--NNN = %d\n", i_status));

	i_status = IniGetInteger("sectionA", "NUM", 99);
	DBGPRINT(("Main -- sectionA--NUM = %d\n", i_status));

	DBGPRINT(("\nMain -- to SetInteger\n"));
	b_status = IniSetInteger("sectionA", "NUM", 12345);
	DBGPRINT(("Main -- NUM set to: %d\n", 12345));
	i_status = IniGetInteger("sectionA", "NUM", 999);
	DBGPRINT(("Main -- sectionA--NUM = %d\n", i_status));

	// 5. get/set value of long type
	l_status = IniGetLong("sectionA", "NNN", 99);
	DBGPRINT(("Main -- sectionA--NNN = %ld\n", l_status));
	l_status = IniGetLong("sectionA", "NUM", 99);
	DBGPRINT(("Main -- sectionA--NUM = %ld\n", l_status));
	DBGPRINT(("\nMain -- to SetInteger\n"));
	b_status = IniSetLong("sectionA", "NUM", 12345678);
	DBGPRINT(("Main -- NUM set to: %ld\n", (long int)12345678));
	l_status = IniGetLong("sectionA", "NUM", 999);
	DBGPRINT(("Main -- sectionA--NUM = %ld\n", l_status));

	// 6. get/set value of double type
	d_status = IniGetDouble("sectionA", "NNN", 99);
	DBGPRINT(("Main -- sectionA--NNN = %g\n", d_status));
	d_status = IniGetDouble("sectionA", "NUM", 99);
	DBGPRINT(("Main -- sectionA--NUM = %g\n", d_status));
	DBGPRINT(("\nMain -- to Set Number"));
	b_status = IniSetDouble("sectionA", "NUM", 12345678.123);
	DBGPRINT(("Main -- NUM set to: %g\n", 12345678.123));
	d_status = IniGetDouble("sectionA", "NUM", 999);
	DBGPRINT(("Main -- sectionA--NUM = %g\n", d_status));

	// 7. get/set value of int type
	b_status = IniGetBool("sectionA", "BOOLA", FALSE);
	DBGPRINT(("Main -- sectionA--BOOLA = %d\n", (int)b_status));
	b_status = IniGetBool("sectionA", "BOOLB", FALSE);
	DBGPRINT(("Main -- sectionA--BOOLB = %d\n", (int)b_status));
	DBGPRINT(("\nMain -- to Set Bool"));
	b_status = IniSetBool("sectionA", "BOOLB", FALSE);
	DBGPRINT(("Main -- BOOLB set to: %d\n", (int)FALSE));
	b_status = IniGetBool("sectionA", "BOOLB", FALSE);
	DBGPRINT(("Main -- sectionA--BOOLB = %d\n", (int)b_status));

	// 8. close file
	IniCloseFile();

	return 0;
}
