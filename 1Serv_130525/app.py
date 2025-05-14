from flask import Flask, request, jsonify
from pymongo import MongoClient
from datetime import datetime, timezone
from flask_cors import CORS

app = Flask(__name__)
CORS(app)

client = MongoClient("")
db = client["iotdb"]
collection = db["gempa"]

@app.route('/data', methods=['POST'])
def receive_data():
    try:
        data = request.json
        document = {
            "richter": float(data["richter"]),
            "mmi": data["mmi"],
            "golongan": data["golongan"],
            "timestamp": datetime.now(timezone.utc)
        }
        collection.insert_one(document)
        return jsonify({"message": "Data saved"}), 201
    except Exception as e:
        return jsonify({"error": str(e)}), 500

@app.route('/data', methods=['GET'])
def get_all_data():
    data = list(collection.find({}, {"_id": 0}))
    return jsonify(data)

if __name__ == '__main__':
    app.run(debug=True, host='0.0.0.0')
