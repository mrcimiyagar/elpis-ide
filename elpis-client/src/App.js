import React from 'react';
import './App.css';
import config from './config.json'
import {setAppInstance, AppBody} from './Renderer';
import { Theme as UWPThemeProvider, getTheme } from "react-uwp/Theme";

export class App extends React.Component {

  constructor(props) {
    super(props);
    this.state = {
      bodyObj: null,
      loadState: 'loading'
    };
    setAppInstance(this);
    window['app'] = this;
    const options = {
      method: 'post',
      headers: {
        'Accept': 'application/json, text/plain, */*',
        'Content-Type': 'application/json'
      },
      body: JSON.stringify({
        "token": "..."
      })
    };
    fetch(config.url, options)
      .then(rawRes => rawRes.json())
      .then(res => {
        this.setState({bodyObj: res});
      });
  }

  render() {
    let result = (
      <div className="App" style={{
        width: '100%',
        height: '100vh'
      }}>
        {this.state.bodyObj === null ? null : <AppBody body={this.state.bodyObj}/>}
        {this.state.loadState !== 'loading' ? null : <div style={{backgroundColor: '#fff', zIndex: 1000000, position: 'fixed', left: 0, top: 0, width: '100%', height: '100%'}}></div>}
      </div>
    );
    return result;
  }
}
