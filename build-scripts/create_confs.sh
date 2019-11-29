#!/bin/bash
declare -a CNFS=( conf_gfx.h conf_snd.h conf_mus.h conf_chr.c conf_chr.h conf_cus.c conf_cus.h conf_grp.c conf_grp.h conf_e2g.c conf_esg.c conf_e2s.c conf_fnt.c conf_g2s.c conf_g2m.c conf_var.c conf_act.c )
for i in "${CNFS[@]}"
do
   ../build-scripts/create_element_defs.pl $i > $i
done