# lvgl 输入设备

## 遇到的坑
    1. lv_indev_drv_t结构体只配置类型和回调函数就可以使用了，不用全都配置
    2. 事件回调函数里如果有会触发事件回调的代码（如，修改对象会触发对象风格改变事件）一定要加判断（比如：修改对象属性时，先判断是不是非对象风格改变事件产生的回调）或是其他代码。防止无限递归导致内存溢出
    3. 事件回调里不能判断是那个按键被按下，被按下了几次，回调函数会不断被调用
    4. 设置lv_indev_drv_t的long_press_time为非零值可以防止持续触发，长按功能不可用，可能还得配置定时器回调参数
    5. 输入设备返回的数据在回调函数传入参数的param变量中，可能是专门针对espidf设定的
## 步骤
第一步：
    初始化硬件接口
第二步：
    注册输入设备
```C
static lv_indev_drv_t indev_drv;
lv_indev_drv_init(&indev);
indev_drv.type = you_input_device_type;
indev_drv.read_cb = you_input_device_read_function;
lv_indev_t *indev = lv_indev_drv_register(&indev_drv);
```
第三步:
    创建并关联组
```C
lv_group_t *group = lv_group_create();
lv_indev_set_group(indev, group);
lv_group_set_default(group); //设置成默认组,可选
```
第四步：
    将组和组件关联
```C
lv_group_add_obj(lv_group_get_default(),obj);//使用默认组
lv_group_add_obj(group,obj);//使用自定义组
```
    
# 使用

注册事件回调就好啦
```C
lv_obj_add_event_cb(obj, event_cb, event_code,(void *)user_data);
```
键盘类型的输入设备按键按下产生的是LV_EVENT_KEY事件。