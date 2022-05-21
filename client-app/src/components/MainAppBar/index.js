import * as React from "react";
import { styled, alpha } from "@mui/material/styles";
import AppBar from "@mui/material/AppBar";
import Box from "@mui/material/Box";
import Toolbar from "@mui/material/Toolbar";
import IconButton from "@mui/material/IconButton";
import Typography from "@mui/material/Typography";
import InputBase from "@mui/material/InputBase";
import MenuIcon from "@mui/icons-material/Menu";
import SearchIcon from "@mui/icons-material/Search";
import { colors } from "../../App";
import { Tab, Tabs } from "@mui/material";
import { ThemeProvider } from "@emotion/react";
import { createTheme } from "@mui/material/styles";
import RunIcon from "@mui/icons-material/PlayArrow";
import PauseIcon from "@mui/icons-material/Pause";
import StopIcon from "@mui/icons-material/Stop";

const tabTheme = createTheme({
  palette: {
    primary: {
      light: "#fff",
      main: "#fff",
      dark: "#fff",
      contrastText: "#fff",
    },
    secondary: {
      light: "#fff",
      main: "#fff",
      dark: "#fff",
      contrastText: "#fff",
    },
  },
});

const Search = styled("div")(({ theme }) => ({
  position: "relative",
  borderRadius: theme.shape.borderRadius,
  backgroundColor: alpha(theme.palette.common.white, 0.15),
  "&:hover": {
    backgroundColor: alpha(theme.palette.common.white, 0.25),
  },
  marginLeft: 0,
  width: "100%",
  [theme.breakpoints.up("sm")]: {
    marginLeft: theme.spacing(1),
    width: "auto",
  },
}));

const SearchIconWrapper = styled("div")(({ theme }) => ({
  padding: theme.spacing(0, 2),
  height: "100%",
  position: "absolute",
  pointerEvents: "none",
  display: "flex",
  alignItems: "center",
  justifyContent: "center",
}));

const StyledInputBase = styled(InputBase)(({ theme }) => ({
  color: "inherit",
  "& .MuiInputBase-input": {
    padding: theme.spacing(1, 1, 1, 0),
    // vertical padding + font size from searchIcon
    paddingLeft: `calc(1em + ${theme.spacing(4)})`,
    transition: theme.transitions.create("width"),
    width: "100%",
    [theme.breakpoints.up("sm")]: {
      width: "12ch",
      "&:focus": {
        width: "20ch",
      },
    },
  },
}));

export default function MainAppBar() {
  const [value, setValue] = React.useState("one");
  const handleChange = (event, newValue) => {
    setValue(newValue);
  };
  return (
    <Box sx={{ flexGrow: 1 }}>
      <AppBar
        position="static"
        style={{
          backgroundColor: colors.colorDark2,
          borderRadius: "0px 0px 16px 16px",
        }}
      >
        <Toolbar>
          <IconButton
            size="large"
            edge="start"
            color="inherit"
            aria-label="open drawer"
            sx={{ mr: 2 }}
          >
            <MenuIcon />
          </IconButton>
          <Typography
            variant="h6"
            noWrap
            component="div"
            sx={{ display: { xs: "none", sm: "block" } }}
          >
            Elpis IDE
          </Typography>
          <ThemeProvider theme={tabTheme}>
            <Tabs
              value={value}
              onChange={handleChange}
              textColor="secondary"
              indicatorColor="secondary"
              centered
              sx={{ flexGrow: 1, display: { xs: "none", sm: "block" } }}
            >
              <Tab
                value="one"
                label="Item One"
                style={{ color: value === "one" ? "#fff" : "#ddd" }}
              />
              <Tab
                value="two"
                label="Item Two"
                style={{ color: value === "two" ? "#fff" : "#ddd" }}
              />
              <Tab
                value="three"
                label="Item Three"
                style={{ color: value === "three" ? "#fff" : "#ddd" }}
              />
            </Tabs>
          </ThemeProvider>

          <Search style={{marginRight: 16}}>
            <SearchIconWrapper>
              <SearchIcon />
            </SearchIconWrapper>
            <StyledInputBase
              placeholder="Search…"
              inputProps={{ "aria-label": "search" }}
            />
          </Search>

          <div
            style={{
              height: 64,
              width: 224,
              marginRight: -24,
              background: "linear-gradient(to right, #3a7bd5, #00d2ff)",
              borderRadius: '0px 0px 16px 0px',
              paddingTop: 12
            }}
          >
            <IconButton
              style={{
                marginLeft: 33,
              }}
              onClick={() => {
                const options = {
                  method: "post",
                  headers: {
                    Accept: "application/json, text/plain, */*",
                    "Content-Type": "application/json",
                  },
                };
                fetch("../run-program", options).then((res) => {});
              }}
            >
              <RunIcon />
            </IconButton>
            <IconButton
              style={{
                marginLeft: 24,
              }}
              onClick={() => {
                const options = {
                  method: "post",
                  headers: {
                    Accept: "application/json, text/plain, */*",
                    "Content-Type": "application/json",
                  },
                };
                fetch("../kill-current", options).then((res) => {});
              }}
            >
              <PauseIcon />
            </IconButton>
            <IconButton
              style={{
                marginLeft: 24,
              }}
              onClick={() => {
                const options = {
                  method: "post",
                  headers: {
                    Accept: "application/json, text/plain, */*",
                    "Content-Type": "application/json",
                  },
                };
                fetch("../kill-current", options).then((res) => {});
              }}
            >
              <StopIcon />
            </IconButton>
          </div>
        </Toolbar>
      </AppBar>
    </Box>
  );
}
