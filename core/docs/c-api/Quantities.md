

Quantities
====================================================================

```C
#include "Spirit/Quantities.h"
```



### Quantity_Get_Magnetization

```C
void Quantity_Get_Magnetization(State * state, float m[3], int idx_image=-1, int idx_chain=-1)
```

Total Magnetization



### Quantity_Get_Topological_Charge

```C
float Quantity_Get_Topological_Charge(State * state, int idx_image=-1, int idx_chain=-1)
```

Topological Charge



### Quantity_Get_Grad_Force_MinimumMode

```C
void Quantity_Get_Grad_Force_MinimumMode(State * state, float * gradient, float * eval, float * mode, float * forces, int idx_image=-1, int idx_chain=-1)
```

Minimum mode following information



### Quantity_Get_HTST_Prefactor

```C
float Quantity_Get_HTST_Prefactor(State * state, int idx_image_minimum, int idx_image_sp, int idx_chain=-1)
```

HTST Prefactor for transition from minimum to saddle point.

Note that the method assumes you gave it correct images, where the
gradient is zero and which correspond to a minimum and a saddle point
respectively.
