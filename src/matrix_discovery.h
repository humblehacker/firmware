#ifndef __MATRIX_DISCOVERY_H__
#define __MATRIX_DISCOVERY_H__

#include <LUFA/Drivers/USB/Class/HID.h>

#include "matrix_discovery_defs.h"
#include "hid_usages.h"

void    MatrixDiscovery__init(void);
uint8_t MatrixDiscovery__get_report(USB_KeyboardReport_Data_t *report);
void    MatrixDiscovery__scan_matrix(void);

#endif // __MATRIX_DISCOVERY_H__
