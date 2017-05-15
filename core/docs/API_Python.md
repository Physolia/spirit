SPIRIT API
====================

Python API
----------

A new `State` can be created with

```python
    from spirit import state
    with state.State("") as p_state:
        # do something with p_state
        pass
```

where you can pass a config file specifying your initial system parameters.
If you do not pass a config file, the implemented defaults are used.
*Note that you currently cannot change the geometry of the systems in your state once they are initialized.*

Chain
-----

| Get Info                                                                    | Return         | Description                                                |
| --------------------------------------------------------------------------- | -------------- | ---------------------------------------------------------- |
| `Get_Index( p_state )`                                                      | `Int`          | Get Chain index                                            |
| `Get_NOI( p_state, idx_chain=-1 )`                                          | `Int`          | Get Chain number of images                                 |
| `Get_Rx( p_state, idx_chain=-1 )`                                           | `Array` | Get Rx                                                     |
| `Get_Rx_Interpolated( p_state, idx_chain=-1 )`                              | `array(Float)` | Get Rx interpolated                                        |
| `Get_Energy( p_state, idx_chain=-1 )`                                       | `array(Float)` | Get Energy of every System in Chain                        |
| `Get_Energy_Interpolated( p_state, idx_chain=-1 )`                          | `array(Float)` | Get interpolated Energy of every System in Chain           |

| Image Manipulation                                                          | Return         | Description                                                |
| --------------------------------------------------------------------------- | -------------- | ---------------------------------------------------------- |
| `Next_Image( p_state, idx_chain=-1 )`                                       | `None`         | Switch active to next image of chain                       |
| `Prev_Image( p_state, idx_chain=-1 )`                                       | `None`         | Switch active to previous image of chain                   |
| `Jump_To_Image( p_state, idx_image=-1, idx_chain=-1 )`                      | `None`         | Switch active to specific image of chain                   |
| `Image_to_Clipboard( p_state, idx_image=-1, idx_chain=-1 )`                 | `None`         | Copy active image to clipboard                             |
| `Replace_Image( p_state, idx_image=-1, idx_chain=-1 )`                      | `None`         | Replace active image in chain                              |
| `Insert_Image_Before( p_state, idx_image=-1, idx_chain=-1 )`                | `None`         | Insert clipboard image before image in chain               |
| `Insert_Image_After( p_state, idx_image=-1, idx_chain=-1 )`                 | `None`         | Insert clipboard image after image in chain                |
| `Push_Back( p_state, idx_chain=-1 )`                                        | `None`         | Insert clipboard image at end of chain                     |
| `Delete_Image( p_state, idx_image=-1, idx_chain=-1 )`                       | `None`         | Delete active image                                        |
| `Pop_Back( p_state, idx_chain=-1 )`                                         | `None`         | Delete image at end of chain                               |

| Data                                                                        | Return         | Description                                                |
| --------------------------------------------------------------------------- | -------------- | ---------------------------------------------------------- |
| `Update_Data( p_state, idx_chain=-1 )`                                      | `None`         | Update the chain's data (interpolated energies etc.)       |
| `Setup_Data( p_state, idx_chain=-1 )`                                       | `None`         | Setup the chain's data arrays (when is this necessary?)    |


Constants
---------

| Physical Constants                                                          | Return         | Description                                        |
| --------------------------------------------------------------------------- | -------------- | -------------------------------------------------- |
| `mu_B( )`                                                                   | `Float`        | The Bohr Magneton [meV / T]                        |
| `k_B( )`                                                                    | `Float`        | The Boltzmann constant [meV / K]                   |

Geometry
--------

| Get Geometry parameters                                                         | Return                | Description                                        |
| ------------------------------------------------------------------------------- | --------------------- | -------------------------------------------------- |
| `Get_Bounds( p_state, min, max, idx_image=-1, idx_chain=-1 )`                   | `None`                | Get Bounds                                         |
| `Get_Center( p_state, center, idx_image=-1, idx_chain=-1 )`                     | `None`                | Get Center                                         |
| `Get_Basis_Vectors( p_state, a, b, c, idx_image=-1, idx_chain=-1 )`             | `None`                | Get Basis vectors                                  |
| `Get_N_Cells( p_state, idx_image=-1, idx_chain=-1 )`                            | `Int, Int, Int`       | Get N Cells                                        |
| `Get_Translation_Vectors( p_state, ta, tb, tc, idx_image=-1, idx_chain=-1 )`    | `None`                | Get Translation Vectors                            |
| `Get_Dimensionality( p_state, idx_image=-1, idx_chain=-1 )`                     | `Int`                 | Get Translation Vectors                            |
| `Get_Spin_Positions( p_state, idx_image=-1, idx_chain=-1 )`                     | `Array`               | Get Pointer to Spin Positions                      |

Hamiltonian
-----------

| Set Parameters                                                                  | Return                | Description                                        |
| ------------------------------------------------------------------------------- | --------------------- | -------------------------------------------------- |
| `Set_Field( p_state, magnitude, direction, idx_image=-1, idx_chain=-1 )`        | `None`                | Set external magnetic field                        |
| `Set_Anisotropy(p_state, magnitude, direction, idx_image=-1, idx_chain=-1)`     | `None`                | Set anisotropy                                     |
| `Set_STT(p_state, magnitude, direction, idx_image=-1, idx_chain=-1)`            | `None`                | Set spin transfer torque                           |
| `Set_Temperature(p_state, temperature, idx_image=-1, idx_chain=-1)`             | `None`                | Set temperature                                    |

Log
---

| Log manipulation                                                         | Return    | Description                 |
| ------------------------------------------------------------------------ | --------- | --------------------------- |
| `Send( p_state, level, sender, message, idx_image=-1, idx_chain=-1 )`    | `None`    | Send a Log message          |
| `Append( p_state )`                                                      | `None`    | Append Log to file          |

Parameters
----------

### LLG

| Set LLG Parameters                                                                  | Return        |
| ----------------------------------------------------------------------------------- | ------------- |
| `Set_LLG_Time_Step( p_state, dt, idx_image=-1, idx_chain=-1 )`                      | `None`        |
| `Set_LLG_Damping( p_state, damping, idx_image=-1, idx_chain=-1 )`                   | `None`        |
| `Set_LLG_N_Iterations( p_state, n_iterations, idx_image=-1, idx_chain=-1 )`         | `None`        |
| `Set_LLG_N_Iterations_Log( p_state, n_iterations_log, idx_image=-1, idx_chain=-1 )` | `None`        |

| Get LLG Parameters                                                                  | Return        |
| ----------------------------------------------------------------------------------- | ------------- |
| `Get_LLG_Time_Step( p_state, dt, idx_image=-1, idx_chain=-1 )`                      | `None`        |
| `Get_LLG_Damping( p_state, damping, idx_image=-1, idx_chain=-1 )`                   | `None`        |
| `Get_LLG_N_Iterations( p_state, n_iterations, idx_image=-1, idx_chain=-1 )`         | `Int`         |
| `Get_LLG_N_Iterations_Log( p_state, n_iterations_log, idx_image=-1, idx_chain=-1 )` | `Int`         |

### GNEB

| Set GNEB Parameters                                                                  | Return        |
| ------------------------------------------------------------------------------------ | ------------- |
| `Set_GNEB_Spring_Constant(p_state, c_spring, idx_image=-1, idx_chain=-1)`            | `None`        |
| `Set_GNEB_Climbing_Falling(p_state, image_type, idx_image=-1, idx_chain=-1)`         | `None`        |
| `Set_GNEB_N_Iterations(p_state, n_iterations, idx_image=-1, idx_chain=-1)`           | `None`        |
| `Set_GNEB_N_Iterations_Log(p_state, n_iterations_log, idx_image=-1, idx_chain=-1)`   | `None`        |

| Get GNEB Parameters                                                                     | Return        |
| --------------------------------------------------------------------------------------- | ------------- |
| `Get_GNEB_Spring_Constant( p_state, c_spring, idx_image=-1, idx_chain=-1 )`             | `None`        |
| `Get_GNEB_Climbing_Falling( p_state, climbing, falling, idx_image=-1, idx_chain=-1 )`   | `None`        |
| `Get_GNEB_N_Iterations( p_state, idx_chain=-1 )`                                        | `Int`         |
| `Get_GNEB_N_Iterations_Log( p_state, idx_chain=-1 )`                                    | `Int`         |
| `Get_GNEB_N_Energy_Interpolations( p_state, idx_chain=-1 )`                             | `Int`         |

Simulation
----------

| Simulation state                                                                                                          | Return     |
| ------------------------------------------------------------------------------------------------------------------------- | ---------- |
| `SingleShot( p_state, method_type, optimizer_type, n_iterations=-1, n_iterations_log=-1, idx_image=-1, idx_chain=-1 )`    | `None`     |
| `PlayPause( p_state, method_type, optimizer_type, n_iterations=-1, n_iterations_log=-1, idx_image=-1, idx_chain=-1 )`     | `None`     |
| `Stop_All( p_state )`                                                                                                     | `None`     |
| `Running_LLG( p_state, idx_image=-1, idx_chain=-1 )`                                                                      | `Boolean`  |
| `Running_LLG_Chain( p_state, idx_chain=-1 )`                                                                              | `Boolean`  |
| `Running_LLG_Anywhere( p_state )`                                                                                         | `Boolean`  |
| `Running_GNEB( p_state, idx_chain=-1 )`                                                                                   | `Boolean`  |
| `Running_GNEB_Anywhere( p_state )`                                                                                        | `Boolean`  |
| `Running_MMF( p_state )`                                                                                                  | `Boolean`  |
| `Running_Any( p_state, idx_image=-1, idx_chain=-1 )`                                                                      | `Boolean`  |
| `Running_Any_Anywhere( p_state )`                                                                                         | `Boolean`  |

State
-----

| State manipulation                                                                  | Return     |
| ----------------------------------------------------------------------------------- | ---------- |
| `setup( configfile="", quiet=False )`                                               | `None`     |
| `delete( p_state )`                                                                 | `None`     |

System
------

| System                                                                   | Return     |
| ------------------------------------------------------------------------ | --------   |
| `Get_Index( p_state )`                                                   | `Int`      |
| `Get_NOS( p_state, idx_image=-1, idx_chain=-1 )`                         | `Int`      |
| `Get_Spin_Directions( p_state, idx_image=-1, idx_chain=-1 )`             | `Array`    |
| `Get_Energy( p_state, idx_image=-1, idx_chain=-1 )`                      | `Float`    |
| `Get_Energy_Array( p_state, energies, idx_image=-1, idx_chain=-1 )`      | `None`     |
| `Update_Data( p_state, idx_image=-1, idx_chain=-1 )`                     | `None`     |
| `Print_Energy_Array( p_state, idx_image=-1, idx_chain=-1 )`              | `None`     |

Transition
----------

| Transition options                                                           | Return   | Description                                                                        |
| ---------------------------------------------------------------------------- | -------- | ---------------------------------------------------------------------------------- |
| `Homogeneous( p_state, idx_1, idx_2, idx_chain=-1 )`                         | `None`   | Generate homogeneous transition between two images of a chain                      |
| `Add_Noise_Temperature( p_state, temperature, idx_1, idx_2, idx_chain=-1)`   | `None`   | Add some temperature-scaled noise to a transition between two images of a chain    |
