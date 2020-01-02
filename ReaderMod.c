#include <vitasdk.h>
#include <psp2/kernel/clib.h>
#include <psp2/kernel/modulemgr.h>
#include <psp2/io/fcntl.h>
#include <taihen.h>
#include <string.h>
#include <stdbool.h>

#define auth_path "ux0:data/rddat/settings/fake_auth"
#define sync_path "ux0:data/rddat/settings/auto_sync"
#define guide_path "ux0:data/rddat/settings/guidebook"
#define sample_path "ux0:data/rddat/settings/sample"
#define locux_path "ux0:data/rddat/settings/ux"
#define locuma_path "ux0:data/rddat/settings/uma"
#define locur_path "ux0:data/rddat/settings/ur"
#define locgrw_path "ux0:data/rddat/settings/grw"

void _start() __attribute__((weak, alias("module_start")));

static int getLoc(void)
{
	int loc = 9;
	int dfd;
	dfd = sceIoDopen(locux_path);
	sceIoDclose(dfd);
	if (dfd >= 0)
		loc = 0;
	else
	{
		dfd = sceIoDopen(locuma_path);
		sceIoDclose(dfd);
		if (dfd >= 0)
			loc = 1;
		else
		{
			dfd = sceIoDopen(locur_path);
			sceIoDclose(dfd);
			if (dfd >= 0)
				loc = 2;
			else
			{
				dfd = sceIoDopen(locgrw_path);
				sceIoDclose(dfd);
				if (dfd >= 0)
					loc = 3;
			}
		}
	}
	return loc;
}

static bool getAuth(void)
{
	bool auth = true;
	int dfd;
	dfd = sceIoDopen(auth_path);
	sceIoDclose(dfd);
	if (dfd < 0)
		auth = false;
	return auth;
}

static bool getSync(void)
{
	bool sync = false;
	int dfd;
	dfd = sceIoDopen(sync_path);
	sceIoDclose(dfd);
	if (dfd < 0)
		sync = true;
	return sync;
}

static bool getGuide(void)
{
	bool guide = false;
	int dfd;
	dfd = sceIoDopen(guide_path);
	sceIoDclose(dfd);
	if (dfd < 0)
		guide = true;
	return guide;
}

static bool getSample(void)
{
	bool sample = true;
	int dfd;
	dfd = sceIoDopen(sample_path);
	sceIoDclose(dfd);
	if (dfd < 0)
		sample = false;
	return sample;
}

int module_start(SceSize argc, const void *args) 
{
	tai_module_info_t info;
	info.size = sizeof(info);

	taiGetModuleInfo("ReaderForPSVita_release", &info);

	//System

	//Disable guest check to switch app to "authenticated" state
	if (getAuth())
	{
		char s1[0x01];
		sceClibMemset(s1, 0x00, 0x05);
		taiInjectData(info.modid, 0x00, 0x2F05EC, s1, 0x05);
	}

	//Disable refresh request at startup
	char s2[0x01];
	sceClibMemset(s2, 0xD0, 0x01);
	if (getSync())
	{
		taiInjectData(info.modid, 0x00, 0x109FF7, s2, 0x01);
		taiInjectData(info.modid, 0x00, 0x109FFF, s2, 0x01);
	}

	//Relocate translation files from app:0 to ux0:data/rddat/
	char str1[0x0F] = "ux0:data/rddat/";
	taiInjectData(info.modid, 0x00, 0x2F07A8, str1, 0x0F);
	char str2[0x0E] = "ux0:data/rddat";
	taiInjectData(info.modid, 0x00, 0x2EC94C, str2, 0x0E);

	//Remove Guidebook from the app
	if (getGuide())
	{
		char s3[0x07];
		sceClibMemset(s3, 0x00, 0x07);
		taiInjectData(info.modid, 0x00, 0x2F5AAC, s3, 0x07);
	}

	//Detect and select custom book location
	switch (getLoc())
	{
	case 0:
		taiInjectData(info.modid, 0x00, 0x2F8610, "ux0:book/u/", 0x0B);
		break;
	case 1:
		taiInjectData(info.modid, 0x00, 0x2F8610, "uma0:book/", 0x0A);
		break;
	case 2:
		taiInjectData(info.modid, 0x00, 0x2F8610, "ur0:book/", 0x09);
		break;
	case 3:
		taiInjectData(info.modid, 0x00, 0x2F8610, "grw0:book/", 0x0A);
		break;
	case 9:
		//default addcont0:
		break;
	}

	taiInjectData(info.modid, 0x00, 0x2FF148, s2, 0x01);

	//Spoof addcont book type to sample book type
	if (getSample())
	{
		char s6[0x07] = "addcont";
		taiInjectData(info.modid, 0x00, 0x2F5AA4, s6, 0x07);
	}

	//Count used space in ux0:book/u/
	char s7[0x01] = "u";
	taiInjectData(info.modid, 0x00, 0x2F84A1, s7, 0x01);
	char s8[0x02];
	sceClibMemset(s8, 0x00, 0x02);
	taiInjectData(info.modid, 0x00, 0x2F84A2, s8, 0x02);

	//Translation
	char t1[0x03] = "GRP";
	char t2[0x17] = ".%u.%u  %02u:%02u      ";
	char t3[0x11] = "%04u.%u.%u       ";
	char t4[0x20] = "Sony Computer Entertainment Inc.";
	char t5[0x1F];
	sceClibMemset(t5, 0x00, 0x1F);
	char t6[0x40] = "This guidebook explains how to use Reader for PlayStation®Vita.";
	char t7[0x53];
	sceClibMemset(t7, 0x00, 0x53);
	char t8[0x09] = "Guidebook";
	char t9[0x09];
	sceClibMemset(t9, 0x00, 0x09);
	char t10[0x0A] = "No message";
	char t11[0x23];
	sceClibMemset(t11, 0x00, 0x23);
	char t12[0x24] = " Sony's e-book store                ";
	char t13[0x27] = "Using Reader™ Store                  ";
	char t14[0x1F] = "Browse free books selection    ";
	char t15[0x2A] = "™ official site                         ";

	taiInjectData(info.modid, 0x00, 0x2F5D39, t1, 0x03);
	taiInjectData(info.modid, 0x00, 0x2F784C, t2, 0x17);
	taiInjectData(info.modid, 0x00, 0x2F7864, t3, 0x11);
	taiInjectData(info.modid, 0x00, 0x2F06BC, t4, 0x20);
	taiInjectData(info.modid, 0x00, 0x2F06DB, t5, 0x1F);
	taiInjectData(info.modid, 0x00, 0x2F06FC, t6, 0x40);
	taiInjectData(info.modid, 0x00, 0x2F073F, t7, 0x53);
	taiInjectData(info.modid, 0x00, 0x2F0794, t8, 0x09);
	taiInjectData(info.modid, 0x00, 0x2F079D, t9, 0x09);
	taiInjectData(info.modid, 0x00, 0x2F2958, t10, 0x0A);
	taiInjectData(info.modid, 0x00, 0x2F2962, t11, 0x23);
	taiInjectData(info.modid, 0x00, 0x2F5B2D, t12, 0x24);
	taiInjectData(info.modid, 0x00, 0x2F5B84, t13, 0x27);
	taiInjectData(info.modid, 0x00, 0x2F5CA8, t14, 0x1F);
	taiInjectData(info.modid, 0x00, 0x2F5CE1, t15, 0x2A);

	return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args) 
{
	return SCE_KERNEL_STOP_SUCCESS;
}