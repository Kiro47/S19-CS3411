int dosum(int array[], int arraysize) {
    int sum;
    int i;

    sum = 0;
    for (i = 0; i < arraysize; i++)
    {
        sum += array[i];
    }

    return sum;
}

int domax(int array[], int arraysize) {
    int max;
    int i;

    max = array[0];

    for (i = 1; i < arraysize; i++)
    {
        if (array[i] > max)
        {
            max = array[i];
        }
    }

    return max;
}
