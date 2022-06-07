# Read msg.payload as dictionary {"performance": FLOAT, "fluctuation": FLOAT}
# Control indicator based on that performance and tmp/limit.json

import sys, os, json

isPrint = False

def load_paramjson(file_path="tmp/param.json"):
    working_path = os.path.dirname(__file__)
    project_path = os.path.join(working_path, "../")
    path = os.path.join(project_path, file_path)
    with open(path, "r") as f:
        data = json.load(f)     # {'fluctuation': 1.823281907433354, 'performance': 89.74358974358975}
    
    return data


def read_payload():
    # Read msg.payload as dictionary {"performance": FLOAT, "fluctuation": FLOAT}
    try: 
        obj = sys.argv[1]
    except: 
        obj = load_paramjson(file_path="tmp/param.json")
        if isPrint: print("Use param.json, instead of 'sys.argv[1]'")
    
    return obj


def load_json_obj(file_path="tmp/filename.json"):
    working_path = os.path.dirname(__file__)
    project_path = os.path.join(working_path, "../")
    path = os.path.join(project_path, file_path)
    with open(path, "r") as f:
        data = json.load(f)     # {'limit': {'performance': [], 'fluctuation': []}}
        for val in data.values():
            data = val      # threshold data --> {'performance': [], 'fluctuation': []}
    
    return data     # {'performance': [], 'fluctuation': []}


# Compare two objects in "performance" & "fluctuation" keys
def check_performance_level(result_obj={}, limit_obj={}, key="performance"):
    level = 2
    for i in range( len(limit_obj[key]) ):      # limit_obj[key] = [0, 60, 80, 100] or [0, 5, 5, 100]
        upper = limit_obj[key][i+1]
        lower = limit_obj[key][i]
        value = result_obj[key]

        if (value >= lower and value <= upper):
            msg = key + " = " + str(value) + " in [" + str(lower) + ", " + str(upper) + "]"
            if isPrint: print(msg)
            return level
        
        level -= 1


# Compare two objects in "performance" & "fluctuation" keys
def check_fluctuation_level(result_obj={}, limit_obj={}, key="fluctuation"):
    level = 0       # the difference beetwen check_performance_level is in here
    for i in range( len(limit_obj[key]) ):
        upper = limit_obj[key][i+1]
        lower = limit_obj[key][i]
        value = result_obj[key]

        if (value >= lower and value <= upper):
            msg = key + " = " + str(value) + " in [" + str(lower) + ", " + str(upper) + "]"
            if isPrint: print(msg)
            return level
        
        level += 1      # the difference beetwen check_performance_level is in here


def determine_flag(level_obj={}):
    # compare 2 level_objects, if 1 is higher --> choose the higher level
    # 0:green, 1:yellow, 2:red
    flag = 0
    for key in level_obj.keys():
        if (level_obj[key] > flag):
            flag = level_obj[key]
    
    return flag


def main():
    # Read msg.payload
    result = read_payload()
    result = load_paramjson(file_path="tmp/param.json")
    print(result)

    # Read limit.json
    limit = load_json_obj(file_path="tmp/limit.json")

    level = {"performance": 0, "fluctuation": 0}
    level["performance"] = check_performance_level(result_obj=result, limit_obj=limit)
    level["fluctuation"] = check_fluctuation_level(result_obj=result, limit_obj=limit)
    if isPrint: print(level)

    flag = determine_flag(level_obj=level)
    print(str(flag))


if __name__ == "__main__":
    main()