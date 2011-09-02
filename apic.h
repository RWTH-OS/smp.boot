/* 
 * Copyright 2010 Stefan Lankes, Chair for Operating Systems,
 *                               RWTH Aachen University
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * This file is part of MetalSVM.
 */

#ifndef __ARCH_APIC_H__
#define __ARCH_APIC_H__

#include <stddef.h>


#define MP_FLT_SIGNATURE	0x5f504d5f

/// Local APIC ID Register
#define APIC_ID			0x0020	
/// Local APIC Version Register
#define APIC_VERSION		0x0030	
/// Task Priority Regster
#define APIC_TPR		0x0080	
/// EOI Register
#define APIC_EOI		0x00B0	
/// Spurious Interrupt Vector Register
#define APIC_SVR		0x00F0	
/// Error Status Register
#define APIC_ESR		0x0280
/// Interrupt Command Register [bits 0-31]
#define APIC_ICR1		0x0300	
/// Interrupt Command Register [bits 32-63]
#define APIC_ICR2		0x0310	
/// LVT Timer Register
#define APIC_LVT_T		0x0320	
/// LVT Thermal Sensor Register
#define APIC_LVT_TSR		0x0330
/// LVT Performance Monitoring Counters Register
#define APIC_LVT_PMC		0x0340	
/// LVT LINT0 Register
#define APIC_LINT0		0x0350	
/// LVT LINT1 Register
#define APIC_LINT1		0x0360
/// LVT Error Register
#define APIC_LVT_ER		0x0370	
/// Initial Count Register
#define APIC_ICR		0x0380	
/// Current Count Register
#define APIC_CCR		0x0390	
/// Divide Configuration Register
#define APIC_DCR		0x03E0	

/// Register index: ID
#define IOAPIC_REG_ID		0x0000	
/// Register index: version
#define IOAPIC_REG_VER		0x0001	
/// Redirection table base
#define IOAPIC_REG_TABLE	0x0010	

#define APIC_DEST_SELF		0x40000
#define APIC_DEST_ALLINC	0x80000
#define APIC_DEST_ALLBUT	0xC0000
#define APIC_ICR_RR_MASK	0x30000
#define APIC_ICR_RR_INVALID	0x00000
#define APIC_ICR_RR_INPROG	0x10000
#define APIC_ICR_RR_VALID	0x20000
#define APIC_INT_LEVELTRIG	0x08000
#define APIC_INT_ASSERT		0x04000
#define APIC_ICR_BUSY		0x01000
#define APIC_DEST_LOGICAL	0x00800
#define APIC_DM_FIXED		0x00000
#define APIC_DM_LOWEST		0x00100
#define APIC_DM_SMI		0x00200
#define APIC_DM_REMRD		0x00300
#define APIC_DM_NMI		0x00400
#define APIC_DM_INIT		0x00500
#define APIC_DM_STARTUP		0x00600
#define APIC_DM_EXTINT		0x00700
#define APIC_VECTOR_MASK	0x000FF

/** @brief MP Floating Pointer Structure */
typedef struct {
	uint32_t signature;
	uint32_t mp_config;
	uint8_t length;
	uint8_t version;
	uint8_t checksum;
	uint8_t features[5];
} __attribute__ ((packed)) apic_mp_t;

/** @brief MP Configuration Table */
typedef struct {
	uint32_t signature;
	uint16_t length;
	uint8_t revision;
	uint8_t checksum;
	uint8_t oem_id[8];
	uint8_t product_id[12];
	uint32_t oem_table;
	uint16_t oem_table_size;
	uint16_t entry_count;
	uint32_t lapic;
	uint16_t extended_table_length;
	uint8_t extended_table_checksum;
} __attribute__ ((packed)) apic_config_table_t;

/** @brief APIC Processor Entry */
typedef struct {
	uint8_t type;
	uint8_t id;
	uint8_t version;
	uint8_t cpu_flags;
	uint32_t cpu_signature;
	uint32_t cpu_feature;
} __attribute__ ((packed)) apic_processor_entry_t;

/** @brief IO APIC Entry */
typedef struct {
	uint8_t type;
	uint8_t id;
	uint8_t version;
	uint8_t enabled;
	uint32_t addr;
} __attribute__ ((packed)) apic_io_entry_t;

/** @brief Bus Entry */
typedef struct {
	uint8_t	type;
	uint8_t	bus_id;
	char	name[6];
} __attribute__ ((packed)) apic_bus_entry_t;

/** @brief I/O Interrupt Assignment Entry */
typedef struct {
	uint8_t type;	// type = 3
	uint8_t itype;	// interrupt type
	uint16_t flags;	// flags , PO and EL
	uint8_t src_bus;	// source bus id
	uint8_t src_irq;	// source interrupt (from the old bus)
	uint8_t dest_apic;	// who it gets sent to 0xFF == all
	uint8_t dest_intin;	// which pin it gets sent to on the IO APIC
} __attribute__ ((packed)) apic_ioirq_entry_t;

typedef struct {
	union {
		struct {
			uint32_t vector		:  8,
				delivery_mode	:  3,	/* 000: FIXED
							 * 001: lowest prio
							 * 111: ExtINT
							 */
				dest_mode	:  1,	/* 0: physical, 1: logical */
				delivery_status	:  1, 
				polarity	:  1, 
				irr		:  1, 
				trigger		:  1,	/* 0: edge, 1: level */
				mask		:  1,	/* 0: enabled, 1: disabled */
				__reserved_2	: 15;
		} bitfield;
		uint32_t whole;
	} lower;
	union {
		struct {
			uint32_t __reserved_1		: 24,	
				physical_dest		:  4,
				__reserved_2		:  4;
		} physical;

		struct {
			uint32_t __reserved_1		: 24,
				logical_dest		:  8;
		} logical;
		uint32_t upper;
	} dest;
} __attribute__ ((packed)) ioapic_route_t;

int apic_init(void);
void apic_eoi(void);
uint32_t apic_cpu_id(void);
int apic_calibration(void);
int has_apic(void);
int apic_is_enabled(void);
int ioapic_inton(uint8_t irq, uint8_t apicid);
int ioapic_intoff(uint8_t irq, uint8_t apicid);
int map_apic(void);

#endif
