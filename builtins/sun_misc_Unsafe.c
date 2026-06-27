#include "builtins.h"

void sun_misc_Unsafe_clinit(Class *this)
{
    Object *unsafe_object = object_new(this);
    class_get_static_field(this, "theUnsafe")->value.data.object = unsafe_object;
}

Object *sun_misc_Unsafe_getUnsafe()
{
    return class_get_static_field(classes_get_class("sun/misc/Unsafe"), "theUnsafe")->value.data.object;
}

long sun_misc_Unsafe_objectFieldOffset(Object *this, Object *reflect_field)
{
    Method *name_method = class_get_method(reflect_field->class, "getName", "()Ljava/lang/String;");
    Method *class_method = class_get_method(reflect_field->class, "getDeclaringClass", "()Ljava/lang/Class;");
    Object *name, *declaring_class;

    if (name_method->class->built_in) {
        name = call_native_method(name_method, variant_make_object(reflect_field)).data.object;
        declaring_class = call_native_method(class_method, variant_make_object(reflect_field)).data.object;
    } else {
        name = method_execute(name_method, reflect_field).data.object;
        declaring_class = method_execute(class_method, reflect_field).data.object;
    }

    Class *class = object_get_field(declaring_class, "class")->value.data.class;
    for (int i = 0; i < class->fields->fields_count; i++) {
        Field *field = &class->fields->fields[i];
        if (!strcmp(field->name, name->fields[0].value.data.ref)) {
            // Hmm, now we need to calculate the offset of this field when it will be inside an object.
            long val = offsetof(Object, fields) + (sizeof(Field) * i);
            printf("Calculated offset of field %s in class %s to be %ld\n", field->name, class->name, val);
            return val;
        }
    }

    return -1;
}

int sun_misc_Unsafe_getAndAddInt(Object *this, Object *target, long offset, int value)
{
    Field *field = (Field *)((char *)target + offset);
    int old_value = field->value.data.int_val;
    field->value.data.int_val += value;
    return old_value;
}

long sun_misc_Unsafe_allocateMemory(Object *this, long size)
{
    void *ptr = malloc(size);
    return (long)ptr;
}

void sun_misc_Unsafe_freeMemory(Object *this, long address)
{
    free((void *)address);
}

char sun_misc_Unsafe_getByte(Object *this, long address)
{
    return *(char *)address;
}

void sun_misc_Unsafe_putLong(Object *this, long address, long value)
{
    *(long *)address = value;
}

static builtin_methods methods[] = {
    { "getUnsafe", "()Lsun/misc/Unsafe;", ACC_STATIC, &sun_misc_Unsafe_getUnsafe },
    { "objectFieldOffset", "(Ljava/lang/reflect/Field;)J", 0, &sun_misc_Unsafe_objectFieldOffset },
    { "getAndAddInt", "(Ljava/lang/Object;JI)I", 0, &sun_misc_Unsafe_getAndAddInt },

    { "allocateMemory", "(J)J", 0, &sun_misc_Unsafe_allocateMemory },
    { "freeMemory", "(J)V", 0, &sun_misc_Unsafe_freeMemory },
    { "getByte", "(J)B", 0, &sun_misc_Unsafe_getByte },
    { "putLong", "(JJ)V", 0, &sun_misc_Unsafe_putLong },
    { "<clinit>", "()V", 0, &sun_misc_Unsafe_clinit },
};

static builtin_fields fields[] = {
    { "theUnsafe", "Lsun/misc/Unsafe;", ACC_PRIVATE | ACC_STATIC },
};

builtins sun_misc_Unsafe_builtins = {
    .parent = "java/lang/Object",
    .fields = fields,
    .methods = methods,
    .fields_length = ARRAY_SIZE(fields),
    .methods_length = ARRAY_SIZE(methods),
};