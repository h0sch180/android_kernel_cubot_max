#ifndef BUILD_LK
#include <linux/string.h>
#include <linux/kernel.h>
#endif
#include "lcm_drv.h"

#ifdef BUILD_LK
#include <platform/mt_gpio.h>
#include <platform/mt_i2c.h>
#include <string.h>
#elif defined(BUILD_UBOOT)
#include <asm/arch/mt_gpio.h>
#else
#include <mt-plat/upmu_common.h>
#include <mach/upmu_sw.h>
#include <mach/upmu_hw.h>
#include <mach/gpio_const.h>
#include <mt-plat/mt_gpio.h>
#include <linux/delay.h>
#endif

/*
 *#include <cust_gpio_usage.h>
 *#include <cust_i2c.h>
 *lenovo-sw wuwl10 modify 20150514 for new lcm timming begin
 */
#ifndef GPIO_LCD_BIAS_ENP_PIN
#define GPIO_LCD_BIAS_ENP_PIN  (GPIO15 | 0x80000000)
#endif
#ifndef GPIO_LCD_BIAS_ENN_PIN
#define GPIO_LCD_BIAS_ENN_PIN  (GPIO14 | 0x80000000)
#endif

#ifndef GPIO_LCM_LED_EN
#define GPIO_LCM_LED_EN  (GPIO83 | 0x80000000)
#endif

#ifndef GPIO_LCM_RST
#define GPIO_LCM_RST  (GPIO146 | 0x80000000)
#endif

#ifndef GPIO_DISP_ID0_PIN
#define GPIO_DISP_ID0_PIN (GPIO4 | 0x80000000)
#endif

/*
 *Lenovo-sw wuwl10 add 20150515 for esd recover backlight
 */
#ifndef BUILD_LK
static unsigned int esd_last_backlight_level = 255;
#endif
/*
 * ---------------------------------------------------------------------------
 *  Local Constants
 * ---------------------------------------------------------------------------
 */

#define FRAME_WIDTH  (1080)
#define FRAME_HEIGHT (1920)

#define LCM_ID_NT35596 (0x96)
/*
 * ---------------------------------------------------------------------------
 *  Local Variables
 * ---------------------------------------------------------------------------
 */

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))

#define UDELAY(n) udelay(n)
#define MDELAY(n) mdelay(n)
/*
 *lenovo-sw wuwl10 20150820 add for ms delay
 */
#define MSDELAY(n) mdelay(n)
/*
 * ---------------------------------------------------------------------------
 *  Local Functions
 * ---------------------------------------------------------------------------
 */

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	(lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update))
#define dsi_set_cmdq(pdata, queue_size, force_update)		(lcm_util.dsi_set_cmdq(pdata, queue_size, force_update))
#define wrtie_cmd(cmd)										(lcm_util.dsi_write_cmd(cmd))
#define write_regs(addr, pdata, byte_nums)					(lcm_util.dsi_write_regs(addr, pdata, byte_nums))
#define read_reg(cmd)										(lcm_util.dsi_dcs_read_lcm_reg(cmd))
#define read_reg_v2(cmd, buffer, buffer_size)   			(lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size))

#define dsi_lcm_set_gpio_out(pin, out)						(mt_set_gpio_out(pin, out))
#define dsi_lcm_set_gpio_mode(pin, mode)					(mt_set_gpio_mode(pin, mode))
#define dsi_lcm_set_gpio_dir(pin, dir)						(mt_set_gpio_dir(pin, dir))
#define dsi_lcm_set_gpio_pull_enable(pin, en)				(mt_set_gpio_pull_enable(pin, en))

#define LCM_DSI_CMD_MODE								(0)
#define REGFLAG_DELAY										0xFE
#define REGFLAG_END_OF_TABLE						0xFF


#if 1
#ifndef BUILD_LK
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/i2c.h>
#include <linux/irq.h>
/*
 *#include <linux/jiffies.h>
 */
#include <linux/uaccess.h>
/*
 *#include <linux/delay.h>
 */
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
/*****************************************************************************
 * Define
 *****************************************************************************/

#define I2C_I2C_LCD_BIAS_CHANNEL 2
#define TPS_I2C_BUSNUM  I2C_I2C_LCD_BIAS_CHANNEL/*for I2C channel 0 */
#define I2C_ID_NAME "tps65132"
#define TPS_ADDR 0x36

/*****************************************************************************
 * GLobal Variable
 *****************************************************************************/
static struct i2c_board_info __initdata tps65132_board_info = {I2C_BOARD_INFO(I2C_ID_NAME, TPS_ADDR)};
static struct i2c_client *tps65132_i2c_client = NULL;
static unsigned short force[] = {0,TPS_ADDR,I2C_CLIENT_END,I2C_CLIENT_END};
static const unsigned short * const forces[] = { force, NULL };
/*
 *lenovo-sw wuwl10 add  20150727 for backlihgt ic dimming setp
 */
static int tps65132_dimming_step_old = -1;

/*****************************************************************************
 * Function Prototype
 *****************************************************************************/
static int tps65132_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int tps65132_remove(struct i2c_client *client);
/*****************************************************************************
 * Data Structure
 *****************************************************************************/

struct tps65132_dev	{
	struct i2c_client	*client;

};

static const struct i2c_device_id tps65132_id[] = {
	{ I2C_ID_NAME, 0 },
	{ }
};
static const struct of_device_id lcm_of_match[] = {
	{.compatible = "mediatek,tps65132"},
	{},
};

static struct i2c_driver tps65132_iic_driver = {
	.id_table	= tps65132_id,
	.probe		= tps65132_probe,
	.remove		= tps65132_remove,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= "tps65132",
		.of_match_table = lcm_of_match,
	},
	.address_list = (const unsigned short*) forces,
};

/*****************************************************************************
 * Function
 *****************************************************************************/



static int tps65132_remove(struct i2c_client *client)
{
	printk("tps65132_remove\n");
	tps65132_i2c_client = NULL;
	i2c_unregister_device(client);
	return 0;
}


int tps65132_write_bytes(unsigned char addr, unsigned char value)
{
	int ret = 0;
	struct i2c_client *client = tps65132_i2c_client;
	char write_data[2] = {0};
	write_data[0] = addr;
	write_data[1] = value;
	client->flags = 0;
	ret = i2c_master_send(client, write_data, 2);
	if (ret < 0)
		printk("tps65132 write data fail !!\n");
	return ret ;
}
EXPORT_SYMBOL_GPL(tps65132_write_bytes);
/*
 *lenovo-sw wuwl10 add  20150727 for backlihgt ic dimming setp begin
 */
static int tps65132_read_bytes(unsigned char reg, unsigned char *read_buf)
{
	int ret = 0;
	struct i2c_client *client = tps65132_i2c_client;

	ret = i2c_master_send(client, &reg, 1);

	ret = i2c_master_recv(client, read_buf, 1);
	if (ret < 0)
		printk("[wuwl10] tps65132_read_bytes fail !!\n");
	else
		printk("[wuwl10] tps65132_read_bytes sucess !!\n");

	return ret ;
}

static ssize_t get_backlihgtic_dimming_setp(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	unsigned char read_value;

	tps65132_read_bytes(0x11, &read_value);
	if ((read_value & 0xf1) == 0x41)
		read_value = 8;
	else {
		read_value &= 0x0f;
		read_value >>= 1;
	}
	ret = sprintf(buf, "%d\n", read_value);
	return ret;
}

static ssize_t set_backlihgtic_dimming_setp(struct device *pdev, struct device_attribute *attr,
		const char *buf, size_t size)
{
	unsigned int dimming_level = 0;
	unsigned int reg_value = 0;
	unsigned int retry = 3;
	unsigned char read_value;

	if (kstrtouint(buf, 0, &dimming_level))
		return -EINVAL;

	dimming_level &= 0x0F;
	if (dimming_level > 8)
		dimming_level = 8;

	if (tps65132_dimming_step_old == dimming_level)
		return size;

	if (dimming_level == 8)
		reg_value = 0x41;
	else {
		reg_value = dimming_level & 0x07;
		reg_value <<= 1;
		reg_value |= 0x51;
	}
	while (retry--) {
		tps65132_write_bytes(0x11, reg_value);
		tps65132_read_bytes(0x11, &read_value);
		if (read_value == reg_value)
			break;
		printk("[wuwl10] set_backlihgtic_dimming_setp fail,retry:%d,write:0x%x,read:0x%x \n", retry, reg_value, read_value);
	}
	if (retry == 0)
		return -1;
	printk("[wuwl10] set_backlihgtic_dimming_setp sucess,old:%d,dimming_level:%d\n", tps65132_dimming_step_old, dimming_level);

	tps65132_dimming_step_old = dimming_level;
	return size;
}

static DEVICE_ATTR(dimming_step_time, S_IRUGO | S_IWUSR, get_backlihgtic_dimming_setp, set_backlihgtic_dimming_setp);
/*
 * module load/unload record keeping
 */
static int tps65132_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret = 0;
	printk("tps65132_iic_probe\n");
	printk("TPS: info==>name=%s addr=0x%x\n", client->name, client->addr);
	client->addr = 0x36;
	tps65132_i2c_client  = client;
	ret = device_create_file(&client->dev, &dev_attr_dimming_step_time);
	if (ret < 0)
		pr_err("failed to create dimming_step_time file\n");
	return 0;
}
/*
 *lenovo-sw wuwl10 add  20150727 for backlihgt ic dimming setp end
 */
static int __init tps65132_iic_init(void)
{

	printk("tps65132_iic_init\n");
	i2c_register_board_info(2, &tps65132_board_info, 1);
	printk("tps65132_iic_init2\n");
	if(i2c_add_driver(&tps65132_iic_driver)!=0)
   	{
        printk("tps65132 unable to add i2c driver.\n");
      	return -1;
    }
	printk("tps65132_iic_init success\n");
	return 0;
}

static void __exit tps65132_iic_exit(void)
{
	printk("tps65132_iic_exit\n");
	i2c_del_driver(&tps65132_iic_driver);
}


module_init(tps65132_iic_init);
module_exit(tps65132_iic_exit);

MODULE_AUTHOR("Xiaokuan Shi");
MODULE_DESCRIPTION("MTK TPS65132 I2C Driver");
MODULE_LICENSE("GPL");

#else

#define TPS65132_SLAVE_ADDR_WRITE  0x6C
static struct mt_i2c_t TPS65132_i2c;

static int TPS65132_write_byte(kal_uint8 addr, kal_uint8 value)
{
	kal_uint32 ret_code = I2C_OK;
	kal_uint8 write_data[2];
	kal_uint16 len;

	write_data[0] = addr;
	write_data[1] = value;

	TPS65132_i2c.id = I2C_I2C_LCD_BIAS_CHANNEL;/*I2C2; */
	/* Since i2c will left shift 1 bit, we need to set FAN5405 I2C address to >>1 */
	TPS65132_i2c.addr = (TPS65132_SLAVE_ADDR_WRITE >> 1);
	TPS65132_i2c.mode = ST_MODE;
	TPS65132_i2c.speed = 100;
	len = 2;

	ret_code = i2c_write(&TPS65132_i2c, write_data, len);
	/*
	 *printf("%s: i2c_write: ret_code: %d\n", __func__, ret_code);
	 */

	return ret_code;
}

#endif
#endif
struct LCM_setting_table {
	unsigned cmd;
	unsigned char count;
	unsigned char para_list[64];
};


static struct LCM_setting_table lcm_backlight_level_setting[] = {
	{0x51, 1, {0xFF}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};
/*
 *lenovo wuwl10 20150604 add CUSTOM_LCM_FEATURE begin
 */

static struct LCM_setting_table lcm_cabc_level_setting[] = {
	{0x55, 1, {0x00}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};
#if 0
static struct LCM_setting_table lcm_inverse_off_setting[] = {
	{0x20, 1, {0x00}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};
static struct LCM_setting_table lcm_inverse_on_setting[] = {
	{0x21, 1, {0x00}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};
#endif
/*
 *lenovo wuwl10 20150604 add CUSTOM_LCM_FEATURE begin
 */
static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
	unsigned int i;

	for (i = 0; i < count; i++) {
		unsigned cmd;
		cmd = table[i].cmd;

		switch (cmd) {
		case REGFLAG_DELAY :
			MDELAY(table[i].count);
			break;

		case REGFLAG_END_OF_TABLE :
			break;

		default:
			dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
		}
	}
}

void TC358768_DCS_write_1A_1P(unsigned char cmd, unsigned char para)
{
	unsigned int data_array[16];

	data_array[0] = (0x00023902);
	data_array[1] = (0x00000000 | (para << 8) | (cmd));
	dsi_set_cmdq(data_array, 2, 1);
}

#define TC358768_DCS_write_1A_0P(cmd)		data_array[0]=(0x00000500 | (cmd<<16)); \
											dsi_set_cmdq(data_array, 1, 1);

static void init_lcm_registers(void)
{

	unsigned int data_array[16];

	TC358768_DCS_write_1A_1P(0xFF, 0xEE);/*cmd 3 */
	TC358768_DCS_write_1A_1P(0xFB, 0x01);
	TC358768_DCS_write_1A_1P(0x18, 0x40);
	MSDELAY(10);
	TC358768_DCS_write_1A_1P(0x18, 0x00);
	MSDELAY(20);
	TC358768_DCS_write_1A_1P(0xFF, 0x04);/*cmd 2 page3 */
	TC358768_DCS_write_1A_1P(0xFB, 0x01);/*not reload mtp */
	TC358768_DCS_write_1A_1P(0x07, 0x20);/*pwm freq source 35M */
	TC358768_DCS_write_1A_1P(0x08, 0x37);/*pwm out  freq 2.56k wuwl10 modify */
	TC358768_DCS_write_1A_1P(0x00, 0x00);/*cabc dimming off */

	TC358768_DCS_write_1A_1P(0xFF, 0x05);/*cmd 2 page3 */
	TC358768_DCS_write_1A_1P(0xFB, 0x01);/*not reload mtp */
	TC358768_DCS_write_1A_1P(0xE7, 0x00);/*disable video drop  0x80 */

	TC358768_DCS_write_1A_1P(0xFF, 0xEE);/*cmd 3 */
	TC358768_DCS_write_1A_1P(0xFB, 0x01);/*not reload mtp */
	TC358768_DCS_write_1A_1P(0x7C, 0x31);/*source ot GND */

	TC358768_DCS_write_1A_1P(0xFF, 0x00);
	TC358768_DCS_write_1A_1P(0x35, 0x00);
	TC358768_DCS_write_1A_1P(0x51, 0x00);
	TC358768_DCS_write_1A_1P(0x53, 0x24);/*cabc dimming off */
	TC358768_DCS_write_1A_1P(0x55, 0x03);/*wuwl10 modify for default cabc mov mode */
	TC358768_DCS_write_1A_1P(0x5E, 0x0F);/*wuwl10 modify for setting min cabc 16 */
	TC358768_DCS_write_1A_1P(0xD3, 0x06);/*vsa+vfp */
	TC358768_DCS_write_1A_1P(0xD4, 0x06);

	TC358768_DCS_write_1A_0P(0x11);
	MSDELAY(120);
	TC358768_DCS_write_1A_0P(0x29);
	/*
	 *MDELAY(20);
	 */
}


static void lcm_setbacklight(unsigned int level)
{
#ifdef BUILD_LK
	dprintf(0, "%s, level = %d\n", __func__, level);
#else
	printk("%s tm, level = %d\n", __func__, level);
	if (level == 1)
		level = 2;
/*
 *Lenovo-sw wuwl10 add 20150515 for esd recover backlight
 */
	esd_last_backlight_level = level;
#endif
	/*
	 * Refresh value of backlight level.
	 */
	lcm_backlight_level_setting[0].para_list[0] = level;

	push_table(lcm_backlight_level_setting, sizeof(lcm_backlight_level_setting) / sizeof(struct LCM_setting_table), 1);
}

/*
 *lenovo-sw wuwl10 20150515 add for esd revovery backlight begin
 */
#if 1
static void lcm_esd_recover_backlight(void)
{

	printk("%s tm, level = %d\n", __func__, esd_last_backlight_level);

	/*
	 * Refresh value of backlight level.
	 */
	lcm_backlight_level_setting[0].para_list[0] = esd_last_backlight_level;

	push_table(lcm_backlight_level_setting, sizeof(lcm_backlight_level_setting) / sizeof(struct LCM_setting_table), 1);
}
#endif
/*
 *lenovo-sw wuwl10 20150515 add for esd revovery backlight end
 */

/*
 * ---------------------------------------------------------------------------
 *  LCM Driver Implementations
 * ---------------------------------------------------------------------------
 */

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
	memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}


static void lcm_get_params(LCM_PARAMS *params)
{
	memset(params, 0, sizeof(LCM_PARAMS));

	params->type = LCM_TYPE_DSI;

	params->width = FRAME_WIDTH;
	params->height = FRAME_HEIGHT;
	params->physical_height = 121;
	params->physical_width = 68;
#if (LCM_DSI_CMD_MODE)
	params->dsi.mode = CMD_MODE;
#else
	params->dsi.mode = BURST_VDO_MODE;
#endif

	/*
	 * DSI
	 */
	/* Command mode setting */
	/*
	 *1 Three lane or Four lane
	 */
	params->dsi.LANE_NUM = LCM_FOUR_LANE;
	/*
	 *The following defined the fomat for data coming from LCD engine.
	 */
	params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq = LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding = LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format = LCM_DSI_FORMAT_RGB888;

	params->dsi.PS = LCM_PACKED_PS_24BIT_RGB888;

	params->dsi.vertical_sync_active = 2;
	params->dsi.vertical_backporch = 4;
	params->dsi.vertical_frontporch = 6;
	params->dsi.vertical_active_line = FRAME_HEIGHT;

	params->dsi.horizontal_sync_active = 2;
	params->dsi.horizontal_backporch = 20;/*wuwl10 modify for new param */
	params->dsi.horizontal_frontporch = 90;
	params->dsi.horizontal_active_pixel = FRAME_WIDTH;
	/*params->dsi.pll_select=1;     //0: MIPI_PLL; 1: LVDS_PLL */
	/*
	 * Bit rate calculation
	 *1 Every lane speed
	 */
	params->dsi.PLL_CLOCK = 460;//wuwl10 modify for new param
	/*
	 *params->dsi.clk_lp_per_line_enable= 1;
	 */
	params->dsi.esd_check_enable = 1;
	params->dsi.customization_esd_check_enable = 0;
/*
 *	params->dsi.ssc_disable = 1;
 *	params->dsi.ssc_range = 4;
 *	params->dsi.noncont_clock= TRUE;
	 *params->dsi.pll_div1 = 0;	// div1=0,1,2,3;div1_real=1,2,4,4 ----0: 546Mbps  1:273Mbps
	 *params->dsi.pll_div2 = 0;	// div2=0,1,2,3;div1_real=1,2,4,4
	 *params->dsi.fbk_div = 0x13;	//0x12  // fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)
 */
}

static void lcm_init(void)
{
#if 1
	unsigned char cmd = 0x0;
	unsigned char data = 0xFF;
	int ret = 0;

	cmd = 0x18;
	data = 0x06; /*pwm LSBs */

	dsi_lcm_set_gpio_mode(GPIO_LCM_LED_EN, GPIO_MODE_00);
	dsi_lcm_set_gpio_dir(GPIO_LCM_LED_EN, GPIO_DIR_OUT);
	dsi_lcm_set_gpio_out(GPIO_LCM_LED_EN, GPIO_OUT_ONE);
	MDELAY(1);
#ifdef BUILD_LK
	ret = TPS65132_write_byte(cmd, data);
	if (ret)
		printf("[LK]tps6132----cmd=%0x--i2c write error----\n", cmd);
	else
		printf("[LK]tps6132----cmd=%0x--i2c write success----\n", cmd);
#else
	ret = tps65132_write_bytes(cmd, data);
	if (ret < 0)
		printk("[KERNEL]tps6132---cmd=%0x-- i2c write error-----\n", cmd);
	else
		printk("[KERNEL]tps6132---cmd=%0x-- i2c write success-----\n", cmd);
#endif

	cmd = 0x19;
	data = 0xCC; /*PWM MSBs */
#ifdef BUILD_LK
	ret = TPS65132_write_byte(cmd, data);
	if (ret)
		printf("[LK]tps6132----cmd=%0x--i2c write error----\n", cmd);
	else
		printf("[LK]tps6132----cmd=%0x--i2c write success----\n", cmd);
#else
	ret = tps65132_write_bytes(cmd, data);
	if (ret < 0)
		printk("[KERNEL]tps6132---cmd=%0x-- i2c write error-----\n", cmd);
	else
		printk("[KERNEL]tps6132---cmd=%0x-- i2c write success-----\n", cmd);
#endif

	cmd = 0x11;
	data = 0x41; /*wuwl10 modify for PWM mode 3,ramp disable when resume */
#ifdef BUILD_LK
	ret = TPS65132_write_byte(cmd, data);
	if (ret)
		printf("[LK]tps6132----cmd=%0x--i2c write error----\n", cmd);
	else
		printf("[LK]tps6132----cmd=%0x--i2c write success----\n", cmd);
#else
	ret = tps65132_write_bytes(cmd, data);
	if (ret < 0)
		printk("[KERNEL]tps6132---cmd=%0x-- i2c write error-----\n", cmd);
	else
		printk("[KERNEL]tps6132---cmd=%0x-- i2c write success-----\n", cmd);
#endif

/*
 *lenovo-sw wuwl10 20150706 add for backlight take no effect when level less than 5 begin
 */
	cmd = 0x12;
	data = 0x61; /*PWM mode control pwm hystersis 0 */
#ifdef BUILD_LK
	ret = TPS65132_write_byte(cmd, data);
	if (ret)
		printf("[LK]tps6132----cmd=%0x--i2c write error----\n", cmd);
	else
		printf("[LK]tps6132----cmd=%0x--i2c write success----\n", cmd);
#else
	ret = tps65132_write_bytes(cmd, data);
	if (ret < 0)
		printk("[KERNEL]tps6132---cmd=%0x-- i2c write error-----\n", cmd);
	else
		printk("[KERNEL]tps6132---cmd=%0x-- i2c write success-----\n", cmd);
#endif
/*
 *lenovo-sw wuwl10 20150706 add for backlight take no effect when level less than 5 end
 */

	cmd = 0x13;
	data = 0x6B; /*wuwl10 20150706 add for  PWM switch freq 1M,OVP 25v */
#ifdef BUILD_LK
	ret = TPS65132_write_byte(cmd, data);
	if (ret)
		printf("[LK]tps6132----cmd=%0x--i2c write error----\n", cmd);
	else
		printf("[LK]tps6132----cmd=%0x--i2c write success----\n", cmd);
#else
	ret = tps65132_write_bytes(cmd, data);
	if (ret < 0)
		printk("[KERNEL]tps6132---cmd=%0x-- i2c write error-----\n", cmd);
	else
		printk("[KERNEL]tps6132---cmd=%0x-- i2c write success-----\n", cmd);
#endif

#endif
	dsi_lcm_set_gpio_mode(GPIO_LCM_RST, GPIO_MODE_00);
	dsi_lcm_set_gpio_dir(GPIO_LCM_RST, GPIO_DIR_OUT);
	dsi_lcm_set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ZERO);
	MDELAY(3);
	/*
	 *set avdd enable
	 */
	dsi_lcm_set_gpio_mode(GPIO_LCD_BIAS_ENP_PIN, GPIO_MODE_00);
	dsi_lcm_set_gpio_dir(GPIO_LCD_BIAS_ENP_PIN, GPIO_DIR_OUT);
	dsi_lcm_set_gpio_out(GPIO_LCD_BIAS_ENP_PIN, GPIO_OUT_ONE);
	MDELAY(9);

	/*
	 *set avee enable
	 */
	dsi_lcm_set_gpio_mode(GPIO_LCD_BIAS_ENN_PIN, GPIO_MODE_00);
	dsi_lcm_set_gpio_dir(GPIO_LCD_BIAS_ENN_PIN, GPIO_DIR_OUT);
	dsi_lcm_set_gpio_out(GPIO_LCD_BIAS_ENN_PIN, GPIO_OUT_ONE);
	MSDELAY(12);

	/*
	 *SET_RESET_PIN(1);
	 */
	dsi_lcm_set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ONE);
	MDELAY(2);
	/*
	 *SET_RESET_PIN(0);
	 */
	dsi_lcm_set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ZERO);
	MDELAY(3);
	/*
	 *SET_RESET_PIN(1);
	 */
	dsi_lcm_set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ONE);
	MDELAY(2);

	/*
	 *SET_RESET_PIN(0);
	 */
	dsi_lcm_set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ZERO);
	MDELAY(3);
	/*
	 *SET_RESET_PIN(1);
	 */
	dsi_lcm_set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ONE);
	MSDELAY(15);
	/*
	 *lcm init code
	 */
	init_lcm_registers();

	/*
	 *MDELAY(5);
	 */
}

static void lcm_suspend(void)
{
	unsigned int data_array[16];
	printk("[wj]%s begin.\n", __func__);
	dsi_lcm_set_gpio_mode(GPIO_LCM_LED_EN, GPIO_MODE_00);
	dsi_lcm_set_gpio_dir(GPIO_LCM_LED_EN, GPIO_DIR_OUT);
	dsi_lcm_set_gpio_out(GPIO_LCM_LED_EN, GPIO_OUT_ZERO);
	MDELAY(5);

	data_array[0] = 0x00280500;	/* Display Off */
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(10);

	data_array[0] = 0x00100500;	/* Sleep In */
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(50);

	/*
	 *set avee enable
	 */
	dsi_lcm_set_gpio_mode(GPIO_LCD_BIAS_ENN_PIN, GPIO_MODE_00);
	dsi_lcm_set_gpio_dir(GPIO_LCD_BIAS_ENN_PIN, GPIO_DIR_OUT);
	dsi_lcm_set_gpio_out(GPIO_LCD_BIAS_ENN_PIN, GPIO_OUT_ZERO);
	MDELAY(2);

	/*
	 *set avdd disable
	 */
	dsi_lcm_set_gpio_mode(GPIO_LCD_BIAS_ENP_PIN, GPIO_MODE_00);
	dsi_lcm_set_gpio_dir(GPIO_LCD_BIAS_ENP_PIN, GPIO_DIR_OUT);
	dsi_lcm_set_gpio_out(GPIO_LCD_BIAS_ENP_PIN, GPIO_OUT_ZERO);
	MDELAY(3);

	/*
	 *set reset disable
	 */
	dsi_lcm_set_gpio_mode(GPIO_LCM_RST, GPIO_MODE_00);
	dsi_lcm_set_gpio_dir(GPIO_LCM_RST, GPIO_DIR_OUT);
	dsi_lcm_set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ZERO);
	printk("[wj]%s end.\n", __func__);
}


static void lcm_resume(void)
{
	lcm_init();
}

#if (LCM_DSI_CMD_MODE)
static void lcm_update(unsigned int x, unsigned int y,
		       unsigned int width, unsigned int height)
{
	unsigned int x0 = x;
	unsigned int y0 = y;
	unsigned int x1 = x0 + width - 1;
	unsigned int y1 = y0 + height - 1;

	unsigned char x0_MSB = ((x0 >> 8) & 0xFF);
	unsigned char x0_LSB = (x0 & 0xFF);
	unsigned char x1_MSB = ((x1 >> 8) & 0xFF);
	unsigned char x1_LSB = (x1 & 0xFF);
	unsigned char y0_MSB = ((y0 >> 8) & 0xFF);
	unsigned char y0_LSB = (y0 & 0xFF);
	unsigned char y1_MSB = ((y1 >> 8) & 0xFF);
	unsigned char y1_LSB = (y1 & 0xFF);

	unsigned int data_array[16];

	data_array[0] = 0x00053902;
	data_array[1] =
		(x1_MSB << 24) | (x0_LSB << 16) | (x0_MSB << 8) | 0x2a;
	data_array[2] = (x1_LSB);
	dsi_set_cmdq(data_array, 3, 1);

	data_array[0] = 0x00053902;
	data_array[1] =
		(y1_MSB << 24) | (y0_LSB << 16) | (y0_MSB << 8) | 0x2b;
	data_array[2] = (y1_LSB);
	dsi_set_cmdq(data_array, 3, 1);

	data_array[0] = 0x002c3909;
	dsi_set_cmdq(data_array, 1, 0);

}
#endif

const static unsigned char LCD_MODULE_ID = 0x00;
static unsigned int lcm_compare_id(void)
{
	//unsigned int id = 0;
	unsigned char  id_pin_read = 0;

	mt_set_gpio_mode(GPIO_DISP_ID0_PIN, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_DISP_ID0_PIN, GPIO_DIR_IN);
	id_pin_read = mt_get_gpio_in(GPIO_DISP_ID0_PIN);

#ifdef BUILD_LK
	dprintf(0, "%s,  id nt35596 tm= 0x%x LCD_ID_value=%d \n", __func__, id, id_pin_read);
#endif
	if (LCD_MODULE_ID == id_pin_read)
		return 1;
	else
		return 0;
}
/*
 *lenovo-sw wuwl10 modify 20150514 for new lcm timming end
 */

/*
 *lenovo wuwl10 20150604 add CUSTOM_LCM_FEATURE begin
 */
#ifdef CONFIG_LENOVO_CUSTOM_LCM_FEATURE
static void lcm_set_cabcmode(unsigned int mode)
{
#ifdef BUILD_LK
	dprintf(0, "%s, mode = %d\n", __func__, mode);
#else
	printk("%s, mode = %d\n", __func__, mode);
#endif

	lcm_cabc_level_setting[0].para_list[0] = mode;
	push_table(lcm_cabc_level_setting, sizeof(lcm_cabc_level_setting) / sizeof(struct LCM_setting_table), 1);
}
#endif
/*
 *lenovo wuwl10 20150604 add CUSTOM_LCM_FEATURE end
 */

LCM_DRIVER nt35596_fhd_dsi_vdo_tm_lcm_drv = {
	.name = "nt35596_fhd_dsi_vdo_tm",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params = lcm_get_params,
	.init = lcm_init,
	.suspend = lcm_suspend,
	.resume = lcm_resume,
	.compare_id = lcm_compare_id,
	/*
	 *.init_power		= lcm_init_power,
 *Lenovo-sw wuwl10 add 20150515 for esd recover backlight
	 */
#ifndef BUILD_LK
	.esd_recover_backlight = lcm_esd_recover_backlight,
#endif
	/*
	 *.resume_power = lcm_resume_power,
	 *.suspend_power = lcm_suspend_power,
	 */
	.set_backlight	= lcm_setbacklight,
#ifdef CONFIG_LENOVO_CUSTOM_LCM_FEATURE
	.set_cabcmode = lcm_set_cabcmode,
#endif
#if (LCM_DSI_CMD_MODE)
	.update = lcm_update,
#endif
};
