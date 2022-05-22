import { Paper } from "@mui/material";
import React, { useEffect } from "react";
import CodeEditor from "./components/CodeEditor";
import FileView from "./components/FileView";
import MainAppBar from "./components/MainAppBar";
import ResMonitor from "./components/ResMonitor";
import CompView from './components/CompView';
import Websocket from 'react-websocket';
import PoolConfigurator, { pool, updatePool } from './memory';

export let colors;
export let updateColors;

export let currentItemId;
export let setCurrentItemId;

export let forceUpdateApp;

function useForceUpdate() {
  const [value, setValue] = React.useState(0); // integer state
  return () => setValue(value => ++value); // update the state to force render
}

function App() {
  forceUpdateApp = useForceUpdate();
  const [fileTree, setFileTree] = React.useState({});
  [currentItemId, setCurrentItemId] = React.useState(undefined);
  [colors, updateColors] = React.useState({
    colorDark1: "#1B262C",
    colorDark2: "#0F4C75",
    colorDark3: "#3282B8",
    colorAccent: "#BBE1FA",
  });
  return (
    <div
      style={{
        width: "100%",
        height: "100vh",
        backgroundColor: colors.colorDark1,
      }}
    >
      <PoolConfigurator />
      <Websocket
        url="ws://localhost:3001/"
        onMessage={(data) => {
          let packet = JSON.parse(data);
          if (packet.type === "terminal") {
            //this.terminalLines.push({
              //type: "system",
              //content: packet.content,
            //});
            //this.terminalBottom.scrollIntoView(false);
            //this.setState({ terminalPrompt: packet.path });
            //if (!this.state.terminalIsBusy) {
              //this.setState({ terminalIsBusy: true });
            //}
          } else if (packet.type === "terminal-close") {
            //this.setState({ terminalIsBusy: false });
          } else if (packet.type === "files") {
            setFileTree(packet.tree);
          }
        }}
      />
      <MainAppBar />
      <div
        style={{
          width: "100%",
          height: "calc(100% - 112px)",
          display: "flex",
        }}
      >
        <div style={{ width: 350, height: "100%" }}>
          <Paper
            style={{
              marginTop: 16,
              marginLeft: 8,
              width: "100%",
              height: "50%",
              backgroundColor: colors.colorDark3,
              padding: 16,
              borderRadius: 16,
            }}
          >
            <CompView itemId={currentItemId}/>
          </Paper>
          <Paper
            style={{
              marginTop: 16,
              marginLeft: 8,
              width: "100%",
              height: "50%",
              backgroundColor: colors.colorDark3,
              padding: 16,
              borderRadius: 16,
            }}
          >
            <FileView fileTree={fileTree}/>
          </Paper>
        </div>
        <div
          style={{
            width: "calc(100% - 350px - 16px - 16px)",
            height: "100%",
            marginLeft: 4,
            marginTop: 16,
          }}
        >
          <div
            style={{
              width: "100%",
              height: "55vh",
              marginLeft: 16,
              display: "flex",
            }}
          >
            <Paper
              style={{
                width: "calc(100% - 350px)",
                height: "100%",
                backgroundColor: colors.colorDark3,
                padding: 16,
                borderRadius: 16,
              }}
            >
              {pool === undefined ? null :
                Object.keys(pool).map(path => {
                  return <div style={{width: '100%', height: '100%', display: currentItemId === path ? 'block' : 'none'}}><CodeEditor itemId={path} /></div>;
                })
              }
            </Paper>
            <Paper
              style={{
                width: 350,
                height: "100%",
                backgroundColor: colors.colorDark3,
                padding: 16,
                borderRadius: 16,
                marginLeft: 16,
              }}
            >
              {/*<FileView />*/}
            </Paper>
          </div>
          <div
            style={{
              width: "100%",
              height: "calc(45vh - 112px)",
              marginTop: 16,
              marginLeft: 16,
              display: "flex",
            }}
          >
            <Paper
              style={{
                width: "calc(100% - 450px)",
                height: "100%",
                backgroundColor: colors.colorDark3,
                padding: 16,
                borderRadius: 16,
              }}
            ></Paper>
            <Paper
              style={{
                marginLeft: 12,
                width: 450,
                height: "100%",
                backgroundColor: colors.colorDark3,
                padding: 16,
                borderRadius: 16,
              }}
            >
              <ResMonitor />
            </Paper>
          </div>
        </div>
      </div>
    </div>
  );
}

export default App;
