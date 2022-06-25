from faker import Faker
from faker.generator import random
from faker_vehicle import VehicleProvider
from datetime import datetime

import time

fake = Faker('en_US')
fake.add_provider(VehicleProvider)
Faker.seed(0)

def append_original_data(f):
    with open(file_name, 'a') as f:
        for i in range(10):
            if i == 0:
                plate = "LKY1360"
                rnum = 2
            elif i == 1:
                plate = "HHF6697"
                rnum = 5
            elif i == 2:
                plate = "GVP9164"
                rnum = 8
            elif i == 3:
                plate = "LBX9051"
                rnum = 10
            elif i == 4:
                plate = "06062"
                rnum = 10
            elif i == 5:
                plate = "LBV6157"
                rnum = 10
            elif i == 6:
                plate = "LVH6056"
                rnum = 10
            elif i == 7:
                plate = "ZPV5837"
                rnum = 10
            elif i == 8:
                plate = "ZDE1985"
                rnum = 10
            else:
                plate = fake.license_plate().replace(" ", "")
                plate = plate.replace("-", "")
                rnum = fake.pyint(1, 100)
            output = plate+"\n"
            if rnum < 3:
                status = "Owner Wanted"
            elif rnum < 6:
                status = "Unpaid Fines - Tow"
            elif rnum < 9:
                status = "Stolen"
            else:
                status = "No Wants / Warrants"
            output += status+"\n"
            # output+=fake.date_this_year().strftime("%m/%d/%Y")+"\n"
            output += fake.date_between_dates(date_start=datetime(
                2022, 1, 1), date_end=datetime(2024, 5, 1)).strftime("%m/%d/%Y")+"\n"
            output += fake.name()+"\n"
            # output+=fake.date_of_birth().strftime("%m/%d/%Y")+"\n"
            output += fake.date_between_dates(date_start=datetime(
                1932, 1, 1), date_end=datetime(2004, 1, 1)).strftime("%m/%d/%Y")+"\n"
            output += fake.address()+"\n"
            output += fake.vehicle_year()+"\n"
            output += fake.vehicle_make()+"\n"
            output += fake.vehicle_model()+"\n"
            output += fake.safe_color_name()+"$"
            f.write(output)
            # print(output, end='')

def append_new_data(f):
    # 25,000,000 data
    num = 25
    num = (num * 1000 * 1000) + 1

    gen = (fake.unique.license_plate() for i in range(num))

    i = 1
    for license_plate in gen:
        plate = license_plate.replace(" ", "")
        plate = plate.replace("-", "")
        rnum = fake.pyint(1, 100)
        output = plate+"\n"

        if rnum < 3:
            status = "Owner Wanted"
        elif rnum < 6:
            status = "Unpaid Fines - Tow"
        elif rnum < 9:
            status = "Stolen"
        else:
            status = "No Wants / Warrants"
        output += status+"\n"
        # output+=fake.date_this_year().strftime("%m/%d/%Y")+"\n"
        output += fake.date_between_dates(date_start=datetime(
            2022, 1, 1), date_end=datetime(2024, 5, 1)).strftime("%m/%d/%Y")+"\n"
        output += fake.name()+"\n"
        # output+=fake.date_of_birth().strftime("%m/%d/%Y")+"\n"
        output += fake.date_between_dates(date_start=datetime(
            1932, 1, 1), date_end=datetime(2004, 1, 1)).strftime("%m/%d/%Y")+"\n"
        output += fake.address()+"\n"
        output += fake.vehicle_year()+"\n"
        output += fake.vehicle_make()+"\n"
        output += fake.vehicle_model()+"\n"
        output += fake.safe_color_name()+"$"
        f.write(output)
        if i % 100000 == 0:
            print(f'progress: write {i} data')

        i += 1
        next(gen)

if __name__ == "__main__":
    file_name = 'datafile1.txt'

    with open(file_name, 'w') as f:
        f.write('')

    start_time = time.time()

    with open(file_name, 'a') as f:
        append_new_data(f)
        append_original_data(f)
        print('finish writing ...')

    end_time = time.time()
    elapsed_time = end_time-start_time

    print(f'Generation time: {elapsed_time} seconds')