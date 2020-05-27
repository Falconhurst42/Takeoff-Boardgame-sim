import tkinter as tk
from PIL import Image, ImageTk

def convert_lat(l):
    return (90-float(l))*(502.0/180.0)+10

def convert_long(l):
    return (180+float(l))*(502.0/180.0)+10

class Map(tk.Frame):
    def __init__(self):
        super().__init__()

        self.initMap()

    def initMap(self):
        ## base window
        self.master.title("Take Off")
        self.pack(fill=tk.BOTH, expand=1)

        ## open map image
        self.img = Image.open("rectang-0.png")
        self.equirect = ImageTk.PhotoImage(self.img)

        ## display map image
        canvas = tk.Canvas(self, width=self.img.size[0]+20,
           height=self.img.size[1]+20)
        canvas.create_image(10, 10, anchor=tk.NW, image=self.equirect)
        canvas.pack(fill=tk.BOTH, expand=1)

        ## Get and Display Airports and connections
        map = open("map_data.txt", "r")

        colors = {"R" : "red3", 
                "O" : "coral", 
                "Y" : "gold", 
                "G" : "lime green", 
                "B" : "RoyalBlue1", 
                "P" : "purple3" }        

        for port in map:
            words = port.split(' ')
            if(len(words) >= 3):
                for i in range(3, len(words)-1)[0::3]:
                    canvas.create_line(convert_long(words[2]), convert_lat(words[1]), convert_long(words[i+1]), convert_lat(words[i]), fill=colors[words[i+2].rstrip('\n')])
                canvas.create_text(convert_long(words[2]), convert_lat(words[1]), text=words[0], justify="center")
        
        map.close()

        


def main():

    root = tk.Tk()
    m = Map()
    root.geometry("1103x502")
    root.mainloop()


if __name__ == '__main__':
    main()