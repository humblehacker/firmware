/*
                   The HumbleHacker Keyboard Project
                 Copyright © 2008-2010, David Whetstone
               david DOT whetstone AT humblehacker DOT com

  This file is a part of The HumbleHacker Keyboard Project.

  The HumbleHacker Keyboard Project is free software: you can redistribute
  it and/or modify it under the terms of the GNU General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  The HumbleHacker Keyboard Project is distributed in the hope that it will
  be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
  Public License for more details.

  You should have received a copy of the GNU General Public License along
  with The HumbleHacker Keyboard Project.  If not, see
  <http://www.gnu.org/licenses/>.

*/

#ifndef __MATRIX_DISCOVERY_H__
#define __MATRIX_DISCOVERY_H__

#include <LUFA/Drivers/USB/Class/HID.h>

#include "matrix_discovery_defs.h"

void    MatrixDiscovery__init(void);
uint8_t MatrixDiscovery__get_report(USB_KeyboardReport_Data_t *report);
void    MatrixDiscovery__scan_matrix(void);

#endif // __MATRIX_DISCOVERY_H__
