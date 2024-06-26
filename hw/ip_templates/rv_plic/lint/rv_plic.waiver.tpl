# Copyright lowRISC contributors (OpenTitan project).
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0
#
# waiver file for Platform-Level Interrupt Controller

waive -rules ONE_BIT_MEM_WIDTH -location {${module_instance_name}.sv} -regexp {Memory '(claim_re|complete_we)' has} ${"\\"}
      -comment "N_TARGET can be 1."

waive -rules VAR_INDEX_RANGE -location {${module_instance_name}.sv} -regexp {(claim_id|complete_id).* (maximum|minimum) value} ${"\\"}
      -comment "Claim ID is guarded inside target module, complete ID has undeterministic behavior if FW writes OOR value"

waive -rules HIER_NET_NOT_READ -location {${module_instance_name}.sv} -regexp {[Nn]et 'tl_[io]\.[ad]_(address|param|user)} ${"\\"}
      -comment "Register interface doesn't use upper address and param, user filed"

waive -rules EXPLICIT_BITLEN -location {${module_instance_name}_target.sv} -regexp {Bit length .* '1'} ${"\\"}
      -comment "i + 1 is assumed as constant and guarded by SRCW"
waive -rules INTEGER -location {${module_instance_name}_target.sv} -regexp {'i' of type int used as} ${"\\"}
      -comment "int i is static and only assigned to irq_id_next when it hits condition"

waive -rules TWOS_COMP -location {${module_instance_name}_target.sv} -regexp {Explicit two's complement with terms} ${"\\"}
      -comment "This is permissible in this context"
