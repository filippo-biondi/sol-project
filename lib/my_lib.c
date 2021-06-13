int timecmp(timespec t1, timespec t2)
{
    if (t1.tv_sec == t2.tv_sec)
        return t1.tv_nsec - t2.tv_nsec;
    else
        return lhs.tv_sec - rhs.tv_sec;
}
