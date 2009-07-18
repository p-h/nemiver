#include <string.h>

typedef struct
{
  int mi0;
  int mi1;
  int mi2;
  int mi3;
  int mi4;
  int mi5;
  int mi6;
  int mi7;
  int mi8;
} big_type_member;

typedef struct
{
  int mi0;
  int mi1;
  int mi2;
  int mi3;
  int mi4;
  int mi5;
  int mi6;
  int mi7;
  int mi8;
  big_type_member btm0;
  big_type_member btm1;
  big_type_member btm2;
  big_type_member btm3;
  big_type_member btm4;
  big_type_member btm5;
  big_type_member btm6;
  big_type_member btm7;
  big_type_member btm8;
  big_type_member btm9;
  big_type_member btm10;
} big_type;

int
main ()
{
  big_type bt;

  memset (&bt, 0, sizeof (typeof (bt)));
  return 0;
}
