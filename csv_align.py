cimport os
import csv
import openpyxl
import keyboard

def extract_data(file_path: str) -> None:
    print(f"正在處理：{file_path}...")
    wb = openpyxl.Workbook()
    ws = wb.active

    with open(file_path, 'r', encoding="utf8") as fin:
        reader = csv.reader(fin, delimiter=',')
        for row in reader:
            if row[0].startswith("517.1"):
                new_row = []
                value_count = len(row)
                for i in range(value_count):
                    mod = i % 6
                    if mod == 1 or mod == 3:
                        new_row.append(row[i])
                    elif mod == 5:
                        new_row.append(row[i])
                        ws.append(new_row)
                        new_row = []
                ws.append(new_row)
    wb.save(file_path + ".xlsx")
    wb.close()
    print(f"已經儲存至：{file_path}.xlsx")
    print("請按 ESC 繼續 . . .")
    while True:
        event = keyboard.read_event()
        if event.name == "esc":
            break

cursor_index = 0
all_files = os.listdir()
csv_files = [f for f in all_files if f.endswith(".csv")]
file_count = len(csv_files)
if file_count == 0:
    print(f"※※※  目前資料夾位置: {os.getcwd()}  ※※※")
    print("※※※  目前資料夾中沒有任何 CSV 檔案，請將 CSV 檔案放到這個資料夾中  ※※※")
    os.system("PAUSE")
    quit()
while True:
    os.system("cls")
    print(f"※※※  目前資料夾位置: {os.getcwd()}  ※※※")
    print("操作說明：按上下鍵可以選擇檔案，按 Enter 鍵可以對選擇的檔案做擷取與換行\n")
    for i in range(file_count):
        if i == cursor_index:
            print(">>", csv_files[i])
        else:
            print("  ", csv_files[i])

    event = keyboard.read_event()
    if event.event_type == keyboard.KEY_UP:
        if event.name == "up":
            cursor_index -= 1
            cursor_index %= file_count
        elif event.name == "down":
            cursor_index += 1
            cursor_index %= file_count
        elif event.name == "enter":
            try:
                extract_data(csv_files[cursor_index])
            except PermissionError:
                print(f"無法存取檔案，請檢查是其他程式是否已經開啟 {csv_files[cursor_index]}")
                print("請按 ESC 繼續 . . .")
                while True:
                    event = keyboard.read_event()
                    if event.name == "esc":
                        break
