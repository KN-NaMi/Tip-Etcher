# 3D Model

*This folder contains the 3D model files for the Tip Etcher assembly. These models are useful for visualization, 3D printing, and mechanical integration.*

## 🔧 FreeCAD Installation and 3D Model Preview

1. **Download the Installer:** Choose the latest stable release.
Go to the official [FreeCAD Downloads](https://www.freecad.org/downloads.php) page and download the version for your operating system (Windows, macOS, or Linux).

2. **Install the Software:**
   * **Windows:** Run the downloaded `.exe` installer and follow the wizard.
   * **macOS:** Open the `.dmg` file and drag the FreeCAD icon into your *Applications* folder.
   * **Linux:** Make the `.AppImage` file executable (`chmod +x`) and double-click to run it.

3. **Open the Assembly File:**
Launch FreeCAD. From the top menu, select **File > Open...** (or press `Ctrl+O` / `Cmd+O`) and locate the main project file. You can find it in our repository at: `./cad/TipEtcherV3.FCStd`.

4. **Navigate the Model:** Camera controls and part visibility.
   * **Rotate view:** Hold `Shift` + `Right Mouse Button` and drag (in default CAD navigation style).
   * **Pan view:** Hold the `Middle Mouse Button` (scroll wheel) and drag.
   * **Hide/Show parts:** Select a specific part or subassembly in the left-hand panel (Tree View) and press `Spacebar`.
     
## ⚙️ Mechanical Design (V3.0 Concept)

This section covers the redesigned mechanical structure and enclosure for the V3.0 device. The primary goal of this revision was to make assembly easier. 

All CAD files are provided in `.FCStd` (FreeCAD).

### 🔩 Bill of Materials (Hardware & Fasteners)

Below is the list of non-printable mechanical components required for the main assembly.

| Component | Specification / Size | Qty | Notes |
| :--- | :--- | :---: | :--- |
| **Screws (Metric)** | M3-(Different sizes) Socket Head | 12 | Used for overall mounting, every hole with *r* = 1.6 is requiring one|
| **Threaded Inserts** | M3 (Brass, Heat-set) | 20 | Some inserts are required to be on the top of each other |

### 🖨️ 3D Printing Guidelines

The custom parts were designed with FDM 3D printing in mind. For the best structural integrity and dimensional accuracy, please follow these slicing recommendations:

* **Recommended Material:** PETG (PLA is not recommended due to low impact resistance and low very flexibility).
* **Layer Height:** 0.2 mm
* **Infill:** 15% - 20% (Grid or Gyroid pattern)
* **Wall Loops / Perimeters:** Minimum 3 walls for structural strength
* **Supports:** Required only for a few parts.

> [!WARNING]  
> **Tolerances:** The design incorporates a 0.2mm clearance for every fitting parts. Ensure your printer is properly calibrated for dimensional accuracy before printing the main chassis.
> **Liquids:** Design is waterproof but only to certain point, authors advice to be careful with any liquids nearby.

### 🛠️ Assembly Notes

1. **Heat-set Inserts:** Begin by pressing the brass inserts into the holes using a soldering iron set to ~250°C. Let the plastic cool completely before inserting screws.
2. **PCB Mounting:** Secure the main board using the M3 screws. *Do not overtighten to avoid cracking the standoffs.*
3. **Design:** Design was optimized for easy maintance in case of any problems with construction.
