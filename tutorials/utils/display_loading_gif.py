import os

from IPython.core.display_functions import display
import ipywidgets


def display_loading_gif():
    import base64
    gif_path = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'loading.gif')
    b64 = base64.b64encode(open(gif_path, 'rb').read()).decode('ascii')
    widget = ipywidgets.HTML(f'<img src="data:image/gif;base64,{b64}" />')
    display(widget)
    return widget