# External data routing in Hydrotel

External data routing lets Hydrotel use **NetCDF files** as the source of selected internal hydrological variables (layers production, lateral inflows, or vertical contribution), then continue the simulation from that point.



---

## 1. Activate the option

In the **simulation file**, add a line that points to the routing configuration file:

```text
EXTERNAL DATA ROUTING;external-data/config.txt
```

- The path may be absolute or relative to the **project directory**.

- Remove the path to the config file (or leave this line out) to run a normal simulation without external data routing.

  

---

## 2. Configuration file format

The configuration file is a plain text file (`key;value`).

| Rule | Description |
|------|-------------|
| Separator | Use `;` between the keyword and its value |
| Comments | Lines starting with `//` are ignored |
| Activation | Uncomment the lines for the mode you want; comment out the others with `//` |



---

## 3. Choose one variable mode

You must enable **exactly one** variable group. Mixing groups is not allowed.

| Mode | Keywords to enable (`VAR`) | Units | Meaning |
|------|--------------------|-------|---------|
| Vertical contribution | `VCONT` | mm | Water supply from rain and snowmelt |
| Layer production | `PROD1`, `PROD2`, `PROD3` | mm | Water depth produced by the soil layer (surface, hypodermic, base) |
| Total production | `PRODTOT` | mm | Total water depth produced by the three soil layers |
| Layer lateral inflow | `QLAT1`, `QLAT2`, `QLAT3` | m³/s | Lateral inflow (RHHU) for the soil layer (surface, hypodermic, base) |
| Total lateral inflow | `QLATTOT` | m³/s | Total lateral inflow (RHHU) |

For each enabled variable `VAR`, set:

```text
VAR_SOURCE;path/to/file.nc
VAR_VARNAME;variable_name_in_netcdf
```

Ex:

```
PROD1_SOURCE;external-data/production_surf.nc
PROD1_VARNAME;production_surf
```

- `*_SOURCE` may be absolute or relative to the **project directory**.
- Several variables may share the same NetCDF file (different `*_VARNAME`).

### What Hydrotel skips for each mode

| Mode | Behaviour |
|------|-----------|
| `VCONT` | Snowmelt is skipped; Hydrotel uses the provided water supply and continues with vertical water balance, surface runoff and river network routing. Weather data interpolation will still run while ignoring precipitation data (air temperatures TMin/TMax are needed for the evapotranspiration model) |
| `PROD1`/`PROD2`/`PROD3` or `PRODTOT` | Snowmelt and vertical water balance are skipped; Hydrotel uses the provided production and continues with surface runoff and river network routing |
| `QLAT1`/`QLAT2`/`QLAT3` or `QLATTOT` | Snowmelt, vertical water balance and surface runoff are skipped; Hydrotel injects the lateral inflows into the reaches and continues with river network routing |

When water temperature model is activated, weather data interpolation will run while ignoring precipitation data; air temperatures (TMin/TMax) are needed for the water temperature model. 



---

## 4. Choose one dimension type (`DIMTYPE`)

Uncomment **one** `DIMTYPE` block and its related dimension / coordinate keywords.

### GRID

Data on a regular lon/lat (or x/y) grid (NetCDF format 9.3.1. Orthogonal multidimensional array representation). Hydrotel interpolates grid values to RHHUs.

```text
DIMTYPE;GRID
LON_DIMNAME;x
LAT_DIMNAME;y
LON_VARNAME;x
LAT_VARNAME;y
```

Compatible with: `VCONT`, `PROD1`/`PROD2`/`PROD3`, `PRODTOT`.

### STATION

Data at stations with lon/lat coordinates (NetCDF format H.2.1. Orthogonal multidimensional array representation of time series). Hydrotel interpolates station values to RHHUs.

```text
DIMTYPE;STATION
STATION_DIMNAME;nbstation
LON_VARNAME;lon
LAT_VARNAME;lat
```

Compatible with: `VCONT`, `PROD1`/`PROD2`/`PROD3`, `PRODTOT`.

### RHHU

One value per RHHU. Values are assigned directly to each RHHU (no spatial interpolation).

```text
DIMTYPE;RHHU
ID_DIMNAME;uhrh
ID_VARNAME;iduhrh
```

Compatible with: `VCONT`, `PROD1`/`PROD2`/`PROD3`, `PRODTOT`, `QLAT1`/`QLAT2`/`QLAT3`, `QLATTOT`.

RHHU IDs in the NetCDF file must match the project RHHUs.

### REACH

One value per river reach. Values are assigned directly to each reach (no spatial interpolation).

```text
DIMTYPE;REACH
ID_DIMNAME;reach
ID_VARNAME;idreach
```

Compatible with: `QLAT1`/`QLAT2`/`QLAT3`, `QLATTOT` only.

Reach IDs in the NetCDF file must match the project reaches.



---

## 5. Time dimension (always required)

```text
TIME_DIMNAME;time
TIME_VARNAME;time
```

Names of the NetCDF time dimension and variable. 

The variable must have a "units" attribute with a value in the format "days since yyyy-mm-dd hh:00:00" or "minutes since yyyy-mm-dd hh:00:00". E.g., "days since 1970-01-01 00:00:00".

The NetCDF time series must cover the simulation period at the simulation time step.



---

## 6. Distribution coefficients

When you use a **total** variable (`PRODTOT` or `QLATTOT`), Hydrotel splits the total across the three layers with:

```text
DISTRIBUTION_COEFF1;0.333
DISTRIBUTION_COEFF2;0.333
```

| Coefficient | Applied to |
|-------------|------------|
| `DISTRIBUTION_COEFF1` | Layer 1 (surface) |
| `DISTRIBUTION_COEFF2` | Layer 2 (hypodermic) |
| `1 − DISTRIBUTION_COEFF1 − DISTRIBUTION_COEFF2` | Layer 3 (base) |

Constraints:

- Each coefficient must be between **0 and 1**
- `DISTRIBUTION_COEFF1 + DISTRIBUTION_COEFF2` must be **≤ 1**

These keywords are required for `PRODTOT` / `QLATTOT`, and are not used when you provide the three layers separately (`PROD1`/`PROD2`/`PROD3` or `QLAT1`/`QLAT2`/`QLAT3`).



---

## 7. Worked examples

### Example A — Total production on a grid

Goal: feed total production (`PRODTOT`) from a gridded NetCDF file.

1. In the simulation file:

   ```text
   EXTERNAL DATA ROUTING;external-data/config.txt
   ```

2. In `external-data/config.txt`, the following lines must be present:

   - `TIME_DIMNAME;time`

   - `TIME_VARNAME;time`

   - `DIMTYPE;GRID`

   - `LON_DIMNAME;x`

   - `LAT_DIMNAME;y`

   - `LON_VARNAME;x`

   - `LAT_VARNAME;y`

   - `PRODTOT_SOURCE;external-data/prod.nc`

   - `PRODTOT_VARNAME;prod`

   - `DISTRIBUTION_COEFF1;0.333`

   - `DISTRIBUTION_COEFF2;0.333`

     Other lines in the file (if present) must be comment out with `//` prefix.

Hydrotel will interpolates `prod` from `external-data/prod.nc` to each RHHU, splits it into the three layers using the coefficients and continues the simulation.

### Example B — Layer production by RHHU

Goal: feed the three production layers already available by RHHU.

1. In the simulation file:

   ```text
   EXTERNAL DATA ROUTING;external-data/config.txt
   ```

2. In `config.txt`, the following lines must be present:

   - `TIME_DIMNAME;time`

   - `TIME_VARNAME;time`

   - `DIMTYPE;RHHU`

   - `ID_DIMNAME;uhrh`

   - `ID_VARNAME;iduhrh`

   - `PROD1_SOURCE;external-data/production_surf.nc`

   - `PROD1_VARNAME;production_surf`

   - `PROD2_SOURCE;external-data/production_hypo.nc`

   - `PROD2_VARNAME;production_hypo`

   - `PROD3_SOURCE;external-data/production_base.nc`

   - `PROD3_VARNAME;production_base`

     Other lines in the file (if present) must be comment out with `//` prefix.

Hydrotel assigns each RHHU its three production values from the NetCDF files and continues the simulation.



---

## 8. Compatible combinations

| Variable mode | GRID/STATION | RHHU | REACH |
|---------------|:----:|:----:|:-----:|
| `VCONT` | yes | yes | no |
| `PROD1`/`PROD2`/`PROD3`/`PRODTOT` | yes | yes | no |
| `QLAT1`/`QLAT2`/`QLAT3`/`QLATTOT` | no | yes | yes |
