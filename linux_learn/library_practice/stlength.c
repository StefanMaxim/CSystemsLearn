/**
 * Raw file for creating a library
 */


int stlength(char* name)
{
    int length;

    length = 0;
    if (name == 0)
    {
        return -1;
    }
    while (*name != '\0')
    {
        length += 1;
        name += 1;
    }
    return length;
}