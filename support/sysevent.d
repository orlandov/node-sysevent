#!/usr/sbin/dtrace -s

#pragma D option quiet

BEGIN
{
	printf("%-30s  %-20s  %s\\n", "PUBLISHER", "CLASS",
	    "SUBCLASS");
}

sysevent:::post
/args[0]->ec_name == NULL/
{
	printf("%-30s  %-20s  %s\\n", args[1]->se_publisher,
	    args[1]->se_class, args[1]->se_subclass);
}
