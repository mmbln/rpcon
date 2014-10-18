

void
testfunc (int intval)
{
  int a;

  switch (intval)
    {
    case 1:
      a = 0;
      break;
    case 2:
      a = 89;
      break;
    default:
      a = 6;
      break;

    }
}



int
main ()
{
  int val;

  testfunc (val);
}
