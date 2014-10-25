// crt_access.c
// compile with: /W1
// This example uses _access to check the file named
// crt_ACCESS.C to see if it exists and if writing is allowed.

#include  <io.h>
#include  <stdio.h>
#include  <stdlib.h>

int main( int ac, char ** av )
{
    // Check for existence.
    if( (_access( av[1], 0 )) != -1 )
    {
        printf_s( "File %s exists.\n", av[1] );

    }
}
