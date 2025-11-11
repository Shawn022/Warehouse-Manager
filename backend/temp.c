#include <stdio.h>
struct Item
{
    int id;
    char sku[20];
    char name[50];
    int quantity;
    float price;
    int reorder;
};
struct Reorder
{
    int priority;
    int id;
    char sku[20];
    int qty;
    int eta;
    char date[25];
};
struct Order
{
    int priority;
    int id;
    char sku[20];
    int qty;
    char destination[50];
    char date[25];
};

int main()
{

    struct Item warehouseItems[] = {
        // 🧴 Section A – Personal Care
        {100, "A-100", "ToothBrushes", 120, 9.99, 50},
        {101, "A-101", "ToothPaste", 80, 19.99, 20},
        {102, "A-102", "MouthWash", 35, 14.99, 40},
        {103, "A-103", "Shampoo", 150, 49.99, 30},
        {104, "A-104", "Conditioner", 100, 54.99, 25},
        {105, "A-105", "Face Wash", 90, 39.50, 25},
        {106, "A-106", "Body Lotion", 60, 79.00, 15},
        {107, "A-107", "Soap Bars", 200, 15.00, 50},
        {108, "A-108", "Deodorant", 110, 89.99, 30},
        {109, "A-109", "Hand Sanitizer", 70, 35.00, 20},

        // 🖋️ Section B – Stationery & Office Supplies
        {200, "B-200", "Ballpoint Pens (Pack of 10)", 150, 120.00, 40},
        {201, "B-201", "Gel Pens (Pack of 5)", 130, 160.00, 30},
        {202, "B-202", "A4 Notebook (200 pages)", 180, 90.00, 50},
        {203, "B-203", "Spiral Notebook (100 pages)", 160, 70.00, 40},
        {204, "B-204", "Sticky Notes (Pack)", 100, 50.00, 25},
        {205, "B-205", "Highlighters (Set of 4)", 80, 180.00, 20},
        {206, "B-206", "Permanent Marker", 110, 45.00, 30},
        {207, "B-207", "Whiteboard Marker", 90, 60.00, 25},
        {208, "B-208", "Office Files (Set of 3)", 70, 150.00, 15},
        {209, "B-209", "Desk Organizer", 40, 450.00, 10},
        {210, "B-210", "Printer Paper (500 sheets)", 60, 350.00, 15},
        {211, "B-211", "Stapler", 50, 220.00, 10},
        {212, "B-212", "Staple Pins (Box)", 100, 40.00, 20},
        {213, "B-213", "Correction Pen", 120, 35.00, 30},
        {214, "B-214", "Glue Stick", 140, 30.00, 35},

        // 🧹 Section C – Cleaning & Hygiene
        {300, "C-300", "Dishwash Soap", 35, 14.99, 40},
        {301, "C-301", "Detergent Powder", 150, 180.00, 50},
        {302, "C-302", "Liquid Cleaner", 100, 120.00, 30},
        {303, "C-303", "Dishwash Gel", 90, 85.00, 25},
        {304, "C-304", "Floor Cleaner", 75, 95.00, 20},
        {305, "C-305", "Glass Cleaner", 60, 110.00, 15},
        {306, "C-306", "Toilet Cleaner", 80, 130.00, 20},
        {307, "C-307", "Air Freshener", 55, 160.00, 15},
        {308, "C-308", "Cleaning Cloth", 120, 35.00, 30},
        {309, "C-309", "Scrub Pads", 140, 25.00, 35},

        // 🍿 Section D – Snacks & Beverages
        {400, "D-400", "Potato Chips", 200, 25.00, 60},
        {401, "D-401", "Popcorn", 180, 30.00, 50},
        {402, "D-402", "Soft Drink (500ml)", 120, 40.00, 25},
        {403, "D-403", "Fruit Juice (1L)", 90, 90.00, 20},
        {404, "D-404", "Energy Drink", 75, 120.00, 15},
        {405, "D-405", "Chocolate Bar", 150, 50.00, 30},
        {406, "D-406", "Candy Pack", 210, 15.00, 60},
        {407, "D-407", "Instant Noodles", 140, 25.00, 40},
        {408, "D-408", "Cookies", 160, 40.00, 45},
        {409, "D-409", "Dry Fruits Mix", 50, 350.00, 10},

        // 🪑 Section E – Home & Kitchen
        {500, "E-500", "Steel Spoon Set", 80, 250.00, 20},
        {501, "E-501", "Non-stick Pan", 40, 1200.00, 10},
        {502, "E-502", "Mixer Grinder", 25, 3500.00, 5},
        {503, "E-503", "Water Bottle", 100, 150.00, 30},
        {504, "E-504", "Plastic Container Set", 90, 400.00, 20},
        {505, "E-505", "Cutlery Set", 60, 750.00, 15},
        {506, "E-506", "Kitchen Knife", 70, 300.00, 25},
        {507, "E-507", "Pressure Cooker", 35, 2500.00, 10},
        {508, "E-508", "Electric Kettle", 40, 1800.00, 10},
        {509, "E-509", "Serving Tray", 55, 600.00, 20},

        // 💻 Section F – Electronics & Accessories
        {600, "F-600", "USB Cable", 150, 120.00, 50},
        {601, "F-601", "Wireless Mouse", 80, 600.00, 20},
        {602, "F-602", "Keyboard", 70, 850.00, 15},
        {603, "F-603", "Headphones", 60, 1500.00, 15},
        {604, "F-604", "Bluetooth Speaker", 40, 2200.00, 10},
        {605, "F-605", "Power Bank", 55, 1800.00, 12},
        {606, "F-606", "Smartwatch", 30, 3200.00, 8},
        {607, "F-607", "LED Bulb", 110, 250.00, 30},
        {608, "F-608", "Extension Cord", 90, 450.00, 20},
        {609, "F-609", "Phone Charger", 100, 700.00, 25}};

    struct Order orders[] = {
        {3, 100, "A-100", 5, "Dock A", "2025-10-18"},
        {2, 200, "B-200", 3, "Dock B", "2025-10-18"},
        {1, 300, "C-300", 10, "Outbound 1", "2025-10-19"},
        {3, 300, "C-300", 2, "Outbound 2", "2025-10-20"},
        {2, 300, "C-300", 1, "Retail", "2025-10-21"},
        {1, 400, "D-400", 15, "Supermarket Zone", "2025-10-22"},
        {2, 401, "D-401", 8, "Dock C", "2025-10-22"},
        {3, 500, "E-500", 4, "Kitchen Supplies", "2025-10-23"},
        {1, 601, "F-601", 6, "Electronics Counter", "2025-10-23"},
        {2, 602, "F-602", 3, "Warehouse Outlet", "2025-10-24"}};

    struct Reorder reorders[] = {
        {3, 100, "A-100", 50, 7, "2025-10-25"},
        {3, 100, "A-100", 30, 10, "2025-10-26"},
        {2, 200, "B-200", 40, 14, "2025-11-01"},
        {2, 200, "B-200", 25, 21, "2025-11-08"},
        {1, 300, "C-300", 15, 30, "2025-11-20"},
        {1, 305, "C-305", 60, 12, "2025-11-15"},
        {2, 404, "D-404", 100, 10, "2025-11-10"},
        {3, 500, "E-500", 40, 8, "2025-11-05"},
        {2, 603, "F-603", 25, 15, "2025-11-18"},
        {1, 607, "F-607", 75, 20, "2025-11-22"}};

    int n = sizeof(warehouseItems) / sizeof(warehouseItems[0]);

    FILE *f1 = fopen("saves/inventory.bin", "wb");

    for (int i = 0; i < n; i++){
        fwrite(&warehouseItems[i], sizeof(struct Item), 1, f1);
    }
    fclose(f1);

    FILE *f2 = fopen("saves/orders.bin", "wb");
    for(int i=0;i<10;i++){
        fwrite(&orders[i],sizeof(struct Order),1,f2);
    }
    fclose(f2);

    FILE* f3=fopen("saves/reorders.bin","wb");
    for(int i=0;i<10;i++){
        fwrite(&reorders[i],sizeof(struct Reorder),1,f3);
    }
    fclose(f3);

    return 0;
}