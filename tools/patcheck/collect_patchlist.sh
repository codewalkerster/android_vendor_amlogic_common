#!/bin/bash

g_total_info_count=0
g_total_path_count=0
declare -A g_patch_count_in_path
patchlist_gen_timeout=60
g_timeout=0
g_time_cost=0

g_patch_info_paths=(
"DroidlogicPatch"
"bootloader/uboot-repo/bl2/bin"
"bootloader/uboot-repo/bl30/rtos_sdk/arch/riscv"
"bootloader/uboot-repo/bl30/rtos_sdk/boards/riscv"
"bootloader/uboot-repo/bl30/rtos_sdk/build_system"
"bootloader/uboot-repo/bl30/rtos_sdk/drivers_aocpu"
"bootloader/uboot-repo/bl30/rtos_sdk/kernel/freertos"
"bootloader/uboot-repo/bl30/rtos_sdk/lib/libc"
"bootloader/uboot-repo/bl30/rtos_sdk/products/aocpu"
"bootloader/uboot-repo/bl30/rtos_sdk/scripts"
"bootloader/uboot-repo/bl30/rtos_sdk/soc/riscv"
"bootloader/uboot-repo/bl30/src_ao"
"bootloader/uboot-repo/bl31/bl31_1.3/bin"
"bootloader/uboot-repo/bl31/bl31_2.7/bin"
"bootloader/uboot-repo/bl32/bl32_3.18/bin"
"bootloader/uboot-repo/bl32/bl32_3.8/bin"
"bootloader/uboot-repo/bl33/v2019"
"bootloader/uboot-repo/bl33/v2023"
"bootloader/uboot-repo/bl40/bin"
"bootloader/uboot-repo/fip"
"bootloader/uboot-repo/soc/templates"
"common/common14-5.15/build/bazel_common_rules"
"common/common14-5.15/build/kernel"
"common/common14-5.15/common"
"common/common14-5.15/common-modules/virtual-device"
"common/common14-5.15/common/common_drivers"
"common/common14-5.15/external/bazel-skylib"
"common/common14-5.15/external/python/absl-py"
"common/common14-5.15/external/stardoc"
"common/common14-5.15/gki_image"
"common/common14-5.15/kernel/configs"
"common/common14-5.15/kernel/tests"
"common/common14-5.15/prebuilts/bazel/linux-x86_64"
"common/common14-5.15/prebuilts/build-tools"
"common/common14-5.15/prebuilts/clang-tools"
"common/common14-5.15/prebuilts/clang/host/linux-x86"
"common/common14-5.15/prebuilts/gcc/linux-x86/host/x86_64-linux-glibc2.17-4.8"
"common/common14-5.15/prebuilts/jdk/jdk11"
"common/common14-5.15/prebuilts/kernel-build-tools"
"common/common14-5.15/prebuilts/ndk-r23"
"common/common14-5.15/tools/mkbootimg"
"common/driver_modules/DTVKit/AFD"
"common/driver_modules/adla/driver"
"common/driver_modules/camera"
"common/driver_modules/gpu"
"common/driver_modules/media_modules"
"common/driver_modules/tdk_linuxdriver"
"common/driver_modules/wifi_bt/bt"
"common/driver_modules/wifi_bt/wifi"
"common/driver_modules/wifi_bt/wifi/amlogic/w1/wifi"
"common/driver_modules/wifi_bt/wifi/amlogic/w1u"
"common/driver_modules/wifi_bt/wifi/amlogic/w2"
"common/driver_modules/wifi_bt/wifi/amlogic/w2l"
"common/driver_modules/wifi_bt/wifi/amlogic/wifi_comm"
"common/driver_modules/wifi_bt/wifi/atbm"
"common/driver_modules/wifi_bt/wifi/broadcom/ap6xxx"
"common/driver_modules/wifi_bt/wifi/marvell/sd8987"
"common/driver_modules/wifi_bt/wifi/mtk/drivers/mt7661"
"common/driver_modules/wifi_bt/wifi/mtk/drivers/mt7663u"
"common/driver_modules/wifi_bt/wifi/mtk/drivers/mt7668u"
"common/driver_modules/wifi_bt/wifi/qualcomm/qca6174"
"common/driver_modules/wifi_bt/wifi/realtek/8723bu"
"common/driver_modules/wifi_bt/wifi/realtek/8723du"
"common/driver_modules/wifi_bt/wifi/realtek/8733bu"
"common/driver_modules/wifi_bt/wifi/realtek/8821cu"
"common/driver_modules/wifi_bt/wifi/realtek/8822cs"
"common/driver_modules/wifi_bt/wifi/realtek/8822cu"
"common/driver_modules/wifi_bt/wifi/realtek/8822es"
"common/driver_modules/wifi_bt/wifi/realtek/8852be"
"common/driver_modules/wifi_bt/wifi/realtek/8852bs"
"common/driver_modules/wifi_bt/wifi/unisoc/uwe5621"
"common/project"
"device/amlogic/bluebell_wv4"
"device/amlogic/common"
"device/amlogic/ohm"
"device/amlogic/ohm-kernel"
"device/amlogic/ohm_wv4"
"device/amlogic/ohm_wv4-kernel"
"device/amlogic/ohmcas"
"device/amlogic/ohmcas2"
"device/amlogic/oppen"
"device/amlogic/oppen-kernel"
"device/amlogic/oppen_wv4"
"device/amlogic/oppen_wv4-kernel"
"device/amlogic/oppencas"
"device/amlogic/pascal"
"device/amlogic/pascal-kernel"
"device/amlogic/planck"
"device/amlogic/planck-kernel"
"device/amlogic/planck_wv4"
"device/amlogic/planck_wv4-kernel"
"device/amlogic/qurra"
"device/amlogic/qurra-kernel"
"device/amlogic/raman"
"device/amlogic/raman-kernel"
"device/amlogic/ross"
"device/amlogic/ross-kernel"
"device/amlogic/tyson"
"device/amlogic/yukawa"
"device/amlogic/yukawa-kernel"
"hardware/amlogic"
"hardware/amlogic/cve/driver"
"hardware/amlogic/cve/lib"
"hardware/amlogic/tv"
"vendor/amlogic/bluebell_wv4"
"vendor/amlogic/common"
"vendor/amlogic/common/ASPlayer"
"vendor/amlogic/common/adla_lib"
"vendor/amlogic/common/aml_npu_common"
"vendor/amlogic/common/auto_patch"
"vendor/amlogic/common/external/ffmpeg"
"vendor/amlogic/common/npu"
"vendor/amlogic/common/pre_submit_for_google"
"vendor/amlogic/common/prebuilt/libmedia"
"vendor/amlogic/common/prebuilt/libmediadrm/playready"
"vendor/amlogic/ohm"
"vendor/amlogic/ohm_wv4"
"vendor/amlogic/ohmcas"
"vendor/amlogic/ohmcas2"
"vendor/amlogic/oppen"
"vendor/amlogic/oppen_wv4"
"vendor/amlogic/oppencas"
"vendor/amlogic/pascal"
"vendor/amlogic/planck"
"vendor/amlogic/planck_wv4"
"vendor/amlogic/qurra"
"vendor/amlogic/raman"
"vendor/amlogic/reference"
"vendor/amlogic/reference/aml_mp_sdk"
"vendor/amlogic/reference/apps/AmStreamingInputService"
"vendor/amlogic/reference/apps/DroidTvExtras"
"vendor/amlogic/reference/apps/JDvrLib"
"vendor/amlogic/reference/apps/TvInput"
"vendor/amlogic/reference/external/DTVKit/android-inputsource"
"vendor/amlogic/reference/external/DTVKit/cas_hal"
"vendor/amlogic/reference/external/DTVKit/dtvkit-amlogic"
"vendor/amlogic/reference/external/DTVKit/releaseDTVKit"
"vendor/amlogic/reference/external/DTVKit/releaseDTVKit/ccta"
"vendor/amlogic/reference/external/dvb"
"vendor/amlogic/reference/external/freetype"
"vendor/amlogic/reference/external/libzvbi_src"
"vendor/amlogic/reference/libdmxresconf"
"vendor/amlogic/reference/libdvr"
"vendor/amlogic/reference/libsecdmx_release"
"vendor/amlogic/reference/prebuilt/livetv"
"vendor/amlogic/reference/subtitle"
"vendor/amlogic/reference/tv"
"vendor/amlogic/reference/tv/tvserver"
"vendor/amlogic/ross"
"vendor/amlogic/tyson"
)

function usage()
{
	echo "usage:"
	echo "./collect_patchlist.sh <patch_info_path_list> <output_file>"
	echo "  patch_info_path_list: patch info path list file"
	echo "  output_file: output file"
	echo " "
}

function gettop
{
    local TOPFILE=build/make/core/envsetup.mk
    if [ -n "$TOP" -a -f "$TOP/$TOPFILE" ] ; then
        # The following circumlocution ensures we remove symlinks from TOP.
        (cd "$TOP"; PWD= /bin/pwd)
    else
        if [ -f $TOPFILE ] ; then
            # The following circumlocution (repeated below as well) ensures
            # that we record the true directory name and not one that is
            # faked up with symlink names.
            PWD= /bin/pwd
        else
            local HERE=$PWD
            local T=
            while [ \( ! \( -f $TOPFILE \) \) -a \( "$PWD" != "/" \) ]; do
                \cd ..
                T=`PWD= /bin/pwd -P`
            done
            \cd "$HERE"
            if [ -f "$T/$TOPFILE" ]; then
                echo "$T"
            fi
        fi
    fi
}

function check_time_elapsed()
{
    g_time_cost=$(($(date +%s)-$g_time_start))
	if [ $g_time_cost -ge $patchlist_gen_timeout ]; then
		g_timeout=1
	fi
}

function log()
{
	local total_seconds=$(($(date +%s)-$g_time_start))
	local minutes=$(((total_seconds % 3600) / 60 ))
	local seconds=$((total_seconds % 60))
    printf "[%02d:%02d] $1\n" $minutes $seconds | tee -a $log_file
}

function get_patch_info_by_path()
{
	local path=$1/$2/patchinfo
	local count=0
	local sub_path=$2

	g_patch_count_in_path[$sub_path]=0
	if [ -d $path ]; then
		cd $path
		local count=$(ls * | wc -l)
		g_patch_count_in_path[$sub_path]=$count
		g_total_info_count=$((g_total_info_count+$count))
		echo "repo:$sub_path:${g_patch_count_in_path[$sub_path]}" >> $patchlist_file
		ls * >> $patchlist_file
		check_time_elapsed
	fi
}

function traversal_info_paths()
{
	local i
	if [ "$1" == "" ]; then
		echo "sdk root can not be empty"
	else
		for ((i=0; i<g_total_path_count;i++)); do
			if [ $g_timeout -eq 1 ] || [ $g_total_info_count -gt 50000 ]; then
				break;
			fi
			get_patch_info_by_path $1 "${g_patch_info_paths[$i]}"
			check_time_elapsed
			##log "total patch $g_total_info_count after ${g_patch_info_paths[$i]}"
		done
		echo "repo:all:$g_total_info_count" >> $patchlist_file
		echo "" >> $patchlist_file
	fi

}

function check_patch_count()
{
	local key
	for key in ${g_patch_count_in_path[@]}; do
        echo "$key:${g_patch_count_in_path[$key]}"
    done
}

function is_aml_sdk_root_path()
{
	local path=$1
	local i
	local count=0
	for ((i=0;i<g_total_path_count;i++))
	{
		if [ -d $path/${g_patch_info_paths[$i]} ]; then
			count=$((count+1))
		fi
	}
	local tmp=$((g_total_path_count*2/3))
	if [ $count -gt $tmp ]; then
		log "probably aml_sdk_path:$path,$count/$tmp"
		return 1
	else
		return 0
	fi
}

function get_aml_sdk_roots()
{
	local from=$1
	local path

	is_aml_sdk_root_path $from
	if [ $? -eq 1 ]; then
		aml_sdk_root=$from
		return 1
	fi
	for path in `ls $from`
	do
		is_aml_sdk_root_path $from/$path
		if [ $? -eq 1 ]; then
			aml_sdk_root=$from/$path
			return 1
		fi
	done
	for path in `ls $from`
	do
		if [ -d $from/$path ]; then
			get_aml_sdk_roots $from/$path
			if [ "$aml_sdk_root" != "" ]; then
				return 1
			fi
		fi
	done
}

g_time_start=$(date +%s)
workdir=`pwd`
patchlist_zip=patchinfo_list.zip

patchlist_file_name=patchinfo_list.txt
patchlist_file=$workdir/$patchlist_file_name

echo "patchlist_file:$patchlist_file"
echo "patchlist_zip:$patchlist_zip"
if [ -f $patchlist_file ]; then
	rm $patchlist_file
fi
touch $patchlist_file
if [ -f $patchlist_zip ]; then
	rm $patchlist_zip
fi

##test code start
##for ((ttt=0;ttt<50000;ttt++))
##do
##    touch ../../patchinfo/test_$ttt
##done
##test code end

aml_sdk_root=""
TOP=$(gettop)
g_total_path_count=${#g_patch_info_paths[@]}
log "TOP:$TOP"
log "start get_aml_sdk_roots,total_path_count:$g_total_path_count"
get_aml_sdk_roots $TOP
aml_sdk_root=$(realpath $aml_sdk_root)
log "aml_sdk_root:$aml_sdk_root"
traversal_info_paths $aml_sdk_root
cd $workdir
if [ "-x$3" == "-x1" ]; then
	check_patch_count $aml_sdk_root
fi
log "zipping..."
zip $patchlist_zip $patchlist_file_name
if [ "-x$2" != "-x" ]; then
	log "mv output to $2"
	mv $patchlist_file $2/
	mv $patchlist_zip $2/
fi
check_time_elapsed
log "collect_patchlist finished cost ${g_time_cost}s,total changeid:$g_total_info_count"
