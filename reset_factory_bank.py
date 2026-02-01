import os
import json

def reset_factory_bank():
    # Detect the Documents folder
    documents = os.path.expanduser("~/Documents")
    bank_path = os.path.join(documents, "NeonSynth", "NeonJr", "Banks", "Factory")
    
    if not os.path.exists(bank_path):
        os.makedirs(bank_path)
        print(f"Created directory: {bank_path}")
    
    patch_names = []
    
    print(f"Generating 128 INIT patches in {bank_path}...")
    
    for i in range(1, 129):
        filename = f"patch_{i}.neon"
        filepath = os.path.join(bank_path, filename)
        
        patch_data = {
            "name": "INIT PATCH",
            "parameters": {}
        }
        
        with open(filepath, 'w') as f:
            json.dump(patch_data, f, indent=4)
        
        patch_names.append("INIT PATCH")
    
    # Update index.txt
    index_path = os.path.join(bank_path, "index.txt")
    with open(index_path, 'w') as f:
        f.write("\n".join(patch_names))
    
    print("Success! Factory bank reset to 128 INIT patches.")

if __name__ == "__main__":
    reset_factory_bank()
